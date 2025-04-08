#ifndef TRACKERMEMORY_H
#define TRACKERMEMORY_H

#include <QObject>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
// Include PCI library headers
#include <pci/pci.h>
#include "trackdata.h"

class TrackerMemory : public QObject
{
    Q_OBJECT
public:
    explicit TrackerMemory(QObject *parent = nullptr);
    ~TrackerMemory();

    // Combined initialization (PCI config + memory mapping)
    bool initialize(uintptr_t baseAddress = 0xdba00000, size_t memSize = 0x0800,
                   uint8_t pciBus = 0x98, uint8_t pciSlot = 0x00, uint8_t pciFunc = 0x00);
    void cleanup();

    // Read status data from the tracker
    bool readStatusData(TrackData& data);

    // Send ping to the tracker
    bool sendPing();

    // Check if the tracker is ready to receive a command
    bool isReadyForCommand();

signals:
    void errorOccurred(const QString& errorMsg);

private:
    int m_fd; // File descriptor for /dev/mem
    void* m_mappedMem; // Mapped memory pointer
    size_t m_memSize; // Size of memory mapping
    uintptr_t m_baseAddress; // Base physical address
    bool m_initialized; // Initialization state

    // PCI device location
    uint8_t m_pciBus;
    uint8_t m_pciSlot;
    uint8_t m_pciFunc;

    // Memory offsets from the PCI interface memory map (Figure B2.7)
    static const size_t STATUS_MESSAGE_OFFSET = 0x0400; // Status Message to Host
    static const size_t COMMAND_MAILBOX_OFFSET = 0x03FE; // Command Mailbox
    static const size_t STATUS_MAILBOX_OFFSET = 0x07FE; // Status Mailbox
    static const size_t QUERY_RESPONSE_MAILBOX_OFFSET = 0x07FC; // Query Response Mailbox
    static const size_t COMMAND_MESSAGE_OFFSET = 0x0000; // Command Message to Tracker

    // PCI configuration method
    bool configurePCIDevice();

    // Helper function to calculate checksum
    uint16_t calculateChecksum(const uint16_t* data, size_t words);

    // Basic read/write operations
    uint16_t readWord(size_t offset);
    void writeWord(size_t offset, uint16_t value);
};

#endif // TRACKERMEMORY_H