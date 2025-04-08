#include "trackermemory.h"
#include <QDebug>
#include <unistd.h>  // For usleep
#include <fcntl.h>   // For open flags
#include <sys/mman.h> // For mmap
#include <cstdlib>   // For system()

TrackerMemory::TrackerMemory(QObject *parent)
    : QObject(parent)
    , m_fd(-1)
    , m_mappedMem(nullptr)
    , m_memSize(0)
    , m_baseAddress(0)
    , m_initialized(false)
    , m_pciBus(0x98)
    , m_pciSlot(0x00)
    , m_pciFunc(0x00)
{
}

TrackerMemory::~TrackerMemory()
{
    cleanup();
}

bool TrackerMemory::configurePCIDevice()
{
    qDebug() << QString("Attempting to configure PCI device at %1:%2.%3")
        .arg(m_pciBus, 2, 16, QChar('0'))
        .arg(m_pciSlot, 2, 16, QChar('0'))
        .arg(m_pciFunc);

    // Format the command string
    QString pciDeviceAddress = QString("%1:%2.%3")
        .arg(m_pciBus, 2, 16, QChar('0'))
        .arg(m_pciSlot, 2, 16, QChar('0'))
        .arg(m_pciFunc);

    QString cmd = QString("setpci -s %1 04.w=0142").arg(pciDeviceAddress);

    qDebug() << "Executing command:" << cmd;

    // Execute the command
    int result = system(cmd.toStdString().c_str());

    if (result != 0) {
        emit errorOccurred(QString("Failed to execute setpci command: %1").arg(cmd));
        return false;
    }

    qDebug() << "Successfully executed:" << cmd;
    return true;
}

bool TrackerMemory::initialize(uintptr_t baseAddress, size_t memSize,
                              uint8_t pciBus, uint8_t pciSlot, uint8_t pciFunc)
{
    // Store PCI device location
    m_pciBus = pciBus;
    m_pciSlot = pciSlot;
    m_pciFunc = pciFunc;

    // First configure the PCI device
    if (!configurePCIDevice()) {
        return false;
    }

    // Open /dev/mem for physical memory access
    m_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (m_fd == -1) {
        emit errorOccurred("Failed to open /dev/mem");
        return false;
    }

    // Map the physical memory
    m_mappedMem = mmap(nullptr, memSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, baseAddress);
    if (m_mappedMem == MAP_FAILED) {
        emit errorOccurred("Failed to map physical memory");
        close(m_fd);
        m_fd = -1;
        return false;
    }

    m_baseAddress = baseAddress;
    m_memSize = memSize;

    // Initialize tracker - critical sequence with delays
    volatile uint16_t* commandMailbox = (volatile uint16_t*)((char*)m_mappedMem + COMMAND_MAILBOX_OFFSET);
    volatile uint16_t* statusMailbox = (volatile uint16_t*)((char*)m_mappedMem + STATUS_MAILBOX_OFFSET);
    volatile uint16_t* queryMailbox = (volatile uint16_t*)((char*)m_mappedMem + QUERY_RESPONSE_MAILBOX_OFFSET);

    qDebug() << "Initializing tracker mailboxes...";

    usleep(100);
    *commandMailbox = 0;
    usleep(100);
    *statusMailbox = 0;
    usleep(100);
    *queryMailbox = 0;
    usleep(100);

    qDebug() << "Tracker initialization complete";

    m_initialized = true;
    return true;
}

void TrackerMemory::cleanup()
{
    if (m_mappedMem != nullptr && m_mappedMem != MAP_FAILED) {
        munmap(m_mappedMem, m_memSize);
        m_mappedMem = nullptr;
    }

    if (m_fd != -1) {
        close(m_fd);
        m_fd = -1;
    }

    m_initialized = false;
}

uint16_t TrackerMemory::readWord(size_t offset)
{
    if (!m_initialized || offset >= m_memSize) {
        return 0;
    }

    volatile uint16_t* ptr = (volatile uint16_t*)((char*)m_mappedMem + offset);
    return *ptr;
}

void TrackerMemory::writeWord(size_t offset, uint16_t value)
{
    if (!m_initialized || offset >= m_memSize) {
        return;
    }

    volatile uint16_t* ptr = (volatile uint16_t*)((char*)m_mappedMem + offset);
    *ptr = value;
}

bool TrackerMemory::isReadyForCommand()
{
    // Check if the command mailbox contains a zero value
    return readWord(COMMAND_MAILBOX_OFFSET) == 0;
}

bool TrackerMemory::sendPing()
{
    if (!m_initialized) {
        emit errorOccurred("Tracker memory not initialized");
        return false;
    }

    // Wait until the tracker is ready to receive a command
    if (!isReadyForCommand()) {
        emit errorOccurred("Tracker not ready for command");
        return false;
    }

    qDebug() << "Sending ping message...";

    // Construct ping message (message type 0)
    uint16_t pingMessage[3];
    pingMessage[0] = 0xA5A5; // Sync word
    pingMessage[1] = 0x0000; // Message Type 0 (Ping)

    // Calculate checksum - two's complement of the sum
    uint16_t sum = 0xA5 + 0xA5; // Sum of bytes, not words
    pingMessage[2] = (~sum) + 1;

    qDebug() << "Ping message checksum:" << QString::number(pingMessage[2], 16);

    // Write the ping message to the command buffer
    for (int i = 0; i < 3; ++i) {
        writeWord(COMMAND_MESSAGE_OFFSET + i*2, pingMessage[i]);
    }

    // Write a non-zero value to the command mailbox to interrupt the tracker
    writeWord(COMMAND_MAILBOX_OFFSET, 1);

    qDebug() << "Waiting for tracker to process command...";

    // Wait for tracker to process (mailbox returns to 0)
    int timeout = 100; // 10 second timeout (100 * 100ms)
    while (readWord(COMMAND_MAILBOX_OFFSET) != 0 && timeout > 0) {
        usleep(100000); // 100ms delay
        timeout--;
    }

    if (timeout <= 0) {
        emit errorOccurred("Timeout waiting for tracker to process command");
        return false;
    }

    qDebug() << "Ping message processed by tracker";
    return true;
}

bool TrackerMemory::readStatusData(TrackData& data)
{
    if (!m_initialized) {
        emit errorOccurred("Tracker memory not initialized");
        return false;
    }

    // Check if there's a new status message available
    if (readWord(STATUS_MAILBOX_OFFSET) == 0) {
        return false; // No new status available
    }

    // Read the entire status message at once (36 bytes / 18 words)
    uint16_t statusMsg[18];
    for (int i = 0; i < 18; ++i) {
        statusMsg[i] = readWord(STATUS_MESSAGE_OFFSET + i*2);
    }

    // Clear the status mailbox to indicate we've read the message
    writeWord(STATUS_MAILBOX_OFFSET, 0);

    // Verify sync word
    if (statusMsg[0] != 0xA5A5) {
        emit errorOccurred("Invalid sync word in status message");
        return false;
    }

    // Verify message type (should be 255)
    if ((statusMsg[1] & 0xFF00) != 0xFF00) {
        emit errorOccurred("Invalid message type in status message");
        return false;
    }

    // Extract data according to Figure B3.1

    // Apply proper scaling for maximum precision (LSB = 1/32 = 0.03125)
    // The card provides raw error values as 16-bit two's complement with scaling factor 1/32
    data.rawErrorX = static_cast<float>(static_cast<int16_t>(statusMsg[2])) / 32.0f;
    data.rawErrorY = static_cast<float>(static_cast<int16_t>(statusMsg[3])) / 32.0f;

    // Decode word 5 (0-indexed, so word 5 is statusMsg[5])
    data.targetPolarity = statusMsg[5] & 0x0007;
    data.trackState = (statusMsg[5] >> 3) & 0x0007;
    data.trackMode = (statusMsg[5] >> 8) & 0x0007;

    // Extract status word
    data.status = statusMsg[6];

    // Target information
    data.targetSizeX = statusMsg[7];
    data.targetSizeY = statusMsg[8];
    data.targetLeft = statusMsg[9];
    data.targetTop = statusMsg[10];
    data.targetPixelCount = statusMsg[11];

    // Mount position (azimuth and elevation are 32-bit values)
    data.azimuth = (statusMsg[13] << 16) | statusMsg[12];
    data.elevation = (statusMsg[15] << 16) | statusMsg[14];

    // Filtered track errors with proper scaling
    data.filteredErrorX = static_cast<float>(static_cast<int16_t>(statusMsg[16])) / 32.0f;
    data.filteredErrorY = static_cast<float>(static_cast<int16_t>(statusMsg[17])) / 32.0f;

    return true;
}

uint16_t TrackerMemory::calculateChecksum(const uint16_t* data, size_t words)
{
    uint16_t sum = 0;

    // Add each byte separately (endian-aware)
    for (size_t i = 0; i < words; ++i) {
        sum += (data[i] >> 8) & 0xFF;  // High byte
        sum += data[i] & 0xFF;         // Low byte
    }

    // Two's complement
    return (~sum) + 1;
}