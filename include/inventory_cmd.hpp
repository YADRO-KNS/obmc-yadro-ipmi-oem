// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2023, KNS Group LLC (YADRO)

#pragma once

#include <ipmid/api-types.hpp>
#include <user_channel/user_layer.hpp>

#include <string>

namespace ipmi
{
namespace yadro
{
namespace inventory
{

// State of the session
#define IPMI_SESSION_BEGIN 1
#define IPMI_SESSION_DATA 2
#define IPMI_SESSION_COMMIT 3

namespace smbios
{

constexpr const char* dbusProperties = "org.freedesktop.DBus.Properties";
constexpr const char* mdrv2Path = "/xyz/openbmc_project/Smbios/MDR_V2";
constexpr const char* mdrv2Interface = "xyz.openbmc_project.Smbios.MDR_V2";

static constexpr const char* mdrType2File = "/var/lib/smbios/smbios2";
static constexpr const char* smbiosPath = "/var/lib/smbios";

static constexpr const uint8_t mdrTypeII = 2; // MDR V2 type

static constexpr const uint8_t mdr2Version = 2;        // MDR V2 versoin
static constexpr const uint8_t smbiosAgentVersion = 1; // Agent version of
                                                       // smbios

struct MDRSMBIOSHeader
{
    uint8_t dirVer;
    uint8_t mdrType;
    uint32_t timestamp;
    uint32_t dataSize;
} __attribute__((packed));
} // namespace smbios

namespace storage
{
// Storage interface types.
#define STORAGE_IFACE_SATA 1
#define STORAGE_IFACE_NVME 2
#define STORAGE_IFACE_SCSI 3

// Storage description.
struct Storage
{
    uint8_t iface;         // Device interface
    uint16_t vendorId;     // Vendor PCIe ID (NVMe only)
    uint64_t size;         // Size in bytes
    char serial[20];       // Serial Number
    char model[40];        // Model
    char path[150];        // Path to device
    uint16_t rotationRate; // Rotation rate (0001h - solid state)
} __attribute__((packed));

constexpr const char* storageMgrPath = "/com/yadro/storage";
constexpr const char* storageMgrInterface = "com.yadro.Storage.Manager";
static constexpr const char* storageDataFile = "/var/lib/inventory/storage.csv";

} // namespace storage

} // namespace inventory
} // namespace yadro
} // namespace ipmi
