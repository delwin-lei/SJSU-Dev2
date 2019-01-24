#pragma once

#include "L1_Drivers/system_timer.hpp"
#include "L1_Drivers/uart.hpp"
#include "utility/macros.hpp"

class LpcFlash
{
public:
  constexpr char kHyperloadFinished   = '*';
  constexpr char kHyperloadReady      = '!';
  constexpr char kChecksumError       = '@';
  constexpr char kOtherError          = '^';
  constexpr uint32_t kBlockSize       = 0x1000;                         // 4096
  constexpr uint32_t kBlocksPerSector = 0x8;                            // 4096
  constexpr uint32_t kSectorSize      = kBlockSize * kBlocksPerSector;  // 32 kB
  constexpr uint32_t kSizeOfSector    = 0x8000;
  constexpr uint32_t kStartSector     = 16;
  constexpr uint32_t kLastSector      = 29;
  constexpr uint32_t kApplicationStartAddress = kStartSector * kBlockSize;
  constexpr uint32_t kFlashDelay              = 10;

  enum IapCommands : uint8_t
  {
    kPrepareFlash           = 50,
    kCopyRamToFlash         = 51,
    kEraseSector            = 52,
    kBlankCheckSector       = 53,
    kReadPartId             = 54,
    kReadBootCode           = 55,
    kReadDeviceSerialNumber = 58,
    kCompare                = 56,
    kReinvokeIsp            = 57
  };

  enum class IapResult : uint32_t
  {
    kCmdSuccess        = 0,
    kInvalidCommand    = 1,
    kSrcAddrError      = 2,
    kDstAddrError      = 3,
    kSrcAddrNotMapped  = 4,
    kDstAddrNotMapped  = 5,
    kCountError        = 6,
    kInvalidSector     = 7,
    kSectorNotBlank    = 8,
    kSectorNotPrepared = 9,
    kCompareError      = 10,
    kBusy              = 11
  };

  const char * const kIapResultString[] = {
    "kCmdSuccess",       "kInvalidCommand",    "kSrcAddrError", "kDstAddrError",
    "kSrcAddrNotMapped", "kDstAddrNotMapped",  "kCountError",   "kInvalidSector",
    "kSectorNotBlank",   "kSectorNotPrepared", "kCompareError", "kBusy"
  };

  SJ2_PACKED(struct) IapCommand_t
  {
    uint32_t command;
    intptr_t parameters[4];
  };

  SJ2_PACKED(struct) IapStatus_t
  {
    IapResult result;
    intptr_t parameters[4];
  };

  SJ2_PACKED(struct) Block_t
  {
    uint8_t data[kBlockSize];
  };

  SJ2_PACKED(struct) Sector_t
  {
    Block_t block[kBlocksPerSector];
  };

  SJ2_PACKED(struct) FlashMemory_t
  {
    // [sector][block][block_bytes]
    Block_t bootloader[kStartSector];
    // [sector][block][block_bytes]
    Sector_t application[kLastSector - kStartSector];
  };

  using IapFunction = void (*)(IapCommand_t *, IapStatus_t *);

// TODO(#177): All of the code below should be moved into the file
// library/L0_LowLevel/lpc_flash.hpp
uint8_t WriteUartToBlock(Block_t * block)
{
  uint32_t checksum = 0;
  for (uint32_t position = 0; position < kBlockSize; position++)
  {
    uint8_t byte          = uart0.Receive(100);
    block->data[position] = byte;
    checksum += byte;
  }
  return static_cast<uint8_t>(checksum & 0xFF);
}

bool WriteUartToRamSector(Sector_t * sector)
{
  bool finished           = false;
  uint32_t blocks_written = 0;
  // Blank RAM sector to all 1s
  memset(sector, 0xFF, sizeof(*sector));
  printf3("Writing to Ram Sector...\n");
  uart0.Send(kHyperloadReady);
  while (blocks_written < kBlocksPerSector)
  {
    uint8_t block_number_msb = uart0.Receive(1000);
    uint8_t block_number_lsb = uart0.Receive(100);
    uint32_t block_number    = (block_number_msb << 8) | block_number_lsb;
    if (0xFFFF == block_number)
    {
      puts3("End Of Blocks\n");
      finished = true;
      break;
    }
    else
    {
      uint32_t partition        = block_number % kBlocksPerSector;
      uint8_t checksum          = WriteUartToBlock(&sector->block[partition]);
      uint8_t expected_checksum = uart0.Receive(1000);
      if (checksum != expected_checksum)
      {
        uart0.Send(kChecksumError);
      }
      else
      {
        blocks_written++;
        if (blocks_written < 8)
        {
          uart0.Send(kHyperloadReady);
        }
      }
    }
  }
  return finished;
}

IapResult PrepareSector(uint32_t start, uint32_t end)
{
  IapCommand_t command  = { 0, { 0 } };
  IapStatus_t status    = { IapResult(0), { 0 } };
  command.command       = IapCommands::kPrepareFlash;
  command.parameters[0] = start;
  command.parameters[1] = end;
  iap(&command, &status);
  return status.result;
}

IapResult EraseSector(uint32_t start, uint32_t end)
{
  IapCommand_t command   = { 0, { 0 } };
  IapStatus_t status     = { IapResult(0), { 0 } };
  IapResult flash_status = PrepareSector(start, end);
  if (flash_status == IapResult::kCmdSuccess)
  {
    command.command       = IapCommands::kEraseSector;
    command.parameters[0] = start;
    command.parameters[1] = end;
    command.parameters[2] = system_controller.GetSystemFrequency() / 1000;
    iap(&command, &status);
  }
  else
  {
    status.result = flash_status;
  }
  return status.result;
}

IapResult FlashBlock(Block_t * block, uint32_t sector_number,
                     uint32_t block_number)
{
  IapCommand_t command   = { 0, { 0 } };
  IapStatus_t status     = { IapResult(0), { 0 } };
  IapResult flash_status = PrepareSector(sector_number, sector_number);
  uint32_t app_sector    = sector_number - kStartSector;
  Block_t * flash_address =
      &(flash->application[app_sector].block[block_number]);
  if (flash_status == IapResult::kCmdSuccess)
  {
    command.command       = IapCommands::kCopyRamToFlash;
    command.parameters[0] = reinterpret_cast<intptr_t>(flash_address);
    command.parameters[1] = reinterpret_cast<intptr_t>(block);
    command.parameters[2] = kBlockSize;
    command.parameters[3] = system_controller.GetSystemFrequency() / 1000;
    iap(&command, &status);
    printf3("Flash Attempted! %p %s\n", flash_address,
            kIapResultString[static_cast<uint32_t>(status.result)]);
  }
  else
  {
    printf3("Flash Failed Preperation 0x%lX!\n",
            kIapResultString[static_cast<uint32_t>(flash_status)]);
    status.result = flash_status;
  }
  return status.result;
}

IapResult VerifySector(Sector_t * ram_sector, uint32_t sector_number)
{
  IapCommand_t command    = { 0, { 0 } };
  IapStatus_t status      = { IapResult(0), { 0 } };
  command.command         = IapCommands::kCompare;
  uint32_t app_sector     = sector_number - kStartSector;
  Block_t * flash_address = &(flash->application[app_sector].block[0]);
  command.parameters[0]   = reinterpret_cast<intptr_t>(ram_sector);
  command.parameters[1]   = reinterpret_cast<intptr_t>(flash_address);
  command.parameters[2]   = kSectorSize;
  iap(&command, &status);
  return status.result;
}

IapResult BlankCheckSector(uint32_t start, uint32_t end)
{
  IapCommand_t command  = { 0, { 0 } };
  IapStatus_t status    = { IapResult(0), { 0 } };
  command.command       = IapCommands::kBlankCheckSector;
  command.parameters[0] = start;
  command.parameters[1] = end;
  iap(&command, &status);
  return status.result;
}

void EraseWithVerifySector(uint32_t sector_number)
{
  printf3("Erasing Flash...\n");
  Delay(kFlashDelay);
  IapResult erase_sector_result, black_check_result;
  do
  {
    erase_sector_result = EraseSector(sector_number, sector_number);
    black_check_result  = BlankCheckSector(sector_number, sector_number);
  } while (erase_sector_result != IapResult::kCmdSuccess ||
           black_check_result != IapResult::kCmdSuccess);
  printf3("Flash Erased and Verified!\n");
}

IapResult FlashSector(Sector_t * ram_sector, uint32_t sector_number,
                      uint32_t blocks_filled_in_sector = kBlocksPerSector)
{
  printf3("Flashing Sector %d\n", sector_number);
  EraseWithVerifySector(sector_number);
  IapResult flash_verified = IapResult::kBusy;
  while (flash_verified != IapResult::kCmdSuccess)
  {
    for (uint32_t block_number = 0; block_number < blocks_filled_in_sector;
         block_number++)
    {
      IapResult block_flashed_successfully = FlashBlock(
          &ram_sector->block[block_number], sector_number, block_number);

      if (block_flashed_successfully != IapResult::kCmdSuccess)
      {
        uint8_t error = static_cast<uint8_t>(block_flashed_successfully);
        printf3("Flash Failed with Code 0x%X!\n", error);
        EraseWithVerifySector(sector_number);
        block_number = 0;
      }
      // Omitting this delay will cause a brown out due to the power
      // consumed during flash programming.
      // The number of blocks flashed without it would vary from 1 to the
      // full sector's amount.
      Delay(kFlashDelay);
    }
    flash_verified = VerifySector(ram_sector, sector_number);
  }
  printf3("Flash Programming Verified\n");
  return IapResult::kCmdSuccess;
  }

};
