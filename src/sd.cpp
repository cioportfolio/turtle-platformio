#include "common.hpp"
#include "sd.hpp"

void initSD() {

  SPI.setRX(SPI_RX);
  SPI.setTX(SPI_TX);
  SPI.setSCK(SPI_SCK);
  SPI.setCS(SD_CS_PIN);
  SPI.begin(SD_CS_PIN);

  SDFSConfig config = SDFSConfig();
  config.setCSPin(SD_CS_PIN);
  SDFS.setConfig(config);
  if (!SDFS.begin()) {
    Serial.print(F(
        "\nSD initialization failed.\n"
        "Do not reformat the card!\n"
        "Is the card correctly inserted?\n"
        "Is there a wiring/soldering problem?\n"));
    if (isSpi(SD_CONFIG)) {
      Serial.print(F(
          "Is SD_CS_PIN set to the correct value?\n"
          "Does another SPI device need to be disabled?\n"));
    }
    return;
  }
}

//------------------------------------------------------------------------------
#ifdef DEBUG
cid_t cid;
csd_t csd;
scr_t scr;
uint8_t cmd6Data[64];
uint32_t eraseSize;
uint32_t ocr;
//------------------------------------------------------------------------------
void cidDmp() {
  cout << F("\nManufacturer ID: ");
  cout << uppercase << showbase << hex << int(cid.mid) << dec << endl;
  cout << F("OEM ID: ") << cid.oid[0] << cid.oid[1] << endl;
  cout << F("Product: ");
  for (uint8_t i = 0; i < 5; i++) {
    cout << cid.pnm[i];
  }
  cout << F("\nRevision: ") << cid.prvN() << '.' << cid.prvM() << endl;
  cout << F("Serial number: ") << hex << cid.psn() << dec << endl;
  cout << F("Manufacturing date: ");
  cout << cid.mdtMonth() << '/' << cid.mdtYear() << endl;
  cout << endl;
}
//------------------------------------------------------------------------------
void printConfig(SdSpiConfig config) {
  cout << F("\nAssuming the SD chip select pin is: ") << int(config.csPin);
  cout << F("\nEdit SD_CS_PIN to change the SD chip select pin.\n");
}
//------------------------------------------------------------------------------
void printConfig(SdioConfig config) {
  (void)config;
  cout << F("Assuming an SDIO interface.\n");
}
//-----------------------------------------------------------------------------
void csdDmp() {
  eraseSize = csd.eraseSize();
  cout << F("cardSize: ") << 0.000512 * csd.capacity();
  cout << F(" MB (MB = 1,000,000 bytes)\n");

  cout << F("flashEraseSize: ") << int(eraseSize) << F(" blocks\n");
  cout << F("eraseSingleBlock: ");
  if (csd.eraseSingleBlock()) {
    cout << F("true\n");
  } else {
    cout << F("false\n");
  }
  cout << F("dataAfterErase: ");
  if (scr.dataAfterErase()) {
    cout << F("ones\n");
  } else {
    cout << F("zeros\n");
  }
}
//------------------------------------------------------------------------------
void errorPrint() {
  if (sd.sdErrorCode()) {
    cout << F("SD errorCode: ") << hex << showbase;
    printSdErrorSymbol(&Serial, sd.sdErrorCode());
    cout << F(" = ") << int(sd.sdErrorCode()) << endl;
    cout << F("SD errorData = ") << int(sd.sdErrorData()) << dec << endl;
  }
}
//------------------------------------------------------------------------------
bool mbrDmp() {
  MbrSector_t mbr;
  bool valid = true;
  if (!sd.card()->readSector(0, (uint8_t *)&mbr)) {
    cout << F("\nread MBR failed.\n");
    errorPrint();
    return false;
  }
  cout << F("\nSD Partition Table\n");
  cout << F("part,boot,bgnCHS[3],type,endCHS[3],start,length\n");
  for (uint8_t ip = 1; ip < 5; ip++) {
    MbrPart_t *pt = &mbr.part[ip - 1];
    if ((pt->boot != 0 && pt->boot != 0X80)) {
      valid = false;
    }
    cout << int(ip) << ',' << uppercase << showbase << hex;
    cout << int(pt->boot) << ',';
    for (int i = 0; i < 3; i++) {
      cout << int(pt->beginCHS[i]) << ',';
    }
    cout << int(pt->type) << ',';
    for (int i = 0; i < 3; i++) {
      cout << int(pt->endCHS[i]) << ',';
    }
    cout << dec << ',';
    cout << getLe32(pt->totalSectors) << endl;
  }
  if (!valid) {
    cout << F("\nMBR not valid, assuming Super Floppy format.\n");
  }
  return true;
}
//------------------------------------------------------------------------------
void dmpVol() {
  cout << F("\nScanning FAT, please wait.\n");
  int32_t freeClusterCount = sd.freeClusterCount();
  if (sd.fatType() <= 32) {
    cout << F("\nVolume is FAT") << int(sd.fatType()) << endl;
  } else {
    cout << F("\nVolume is exFAT\n");
  }
  cout << F("sectorsPerCluster: ") << sd.sectorsPerCluster() << endl;
  cout << F("fatStartSector:    ") << sd.fatStartSector() << endl;
  cout << F("dataStartSector:   ") << sd.dataStartSector() << endl;
  cout << F("clusterCount:      ") << sd.clusterCount() << endl;
  cout << F("freeClusterCount:  ");
  if (freeClusterCount >= 0) {
    cout << freeClusterCount << endl;
  } else {
    cout << F("failed\n");
    errorPrint();
  }
}
//------------------------------------------------------------------------------
void printCardType() {
  cout << F("\nCard type: ");

  switch (sd.card()->type()) {
    case SD_CARD_TYPE_SD1:
      cout << F("SD1\n");
      break;

    case SD_CARD_TYPE_SD2:
      cout << F("SD2\n");
      break;

    case SD_CARD_TYPE_SDHC:
      if (csd.capacity() < 70000000) {
        cout << F("SDHC\n");
      } else {
        cout << F("SDXC\n");
      }
      break;

    default:
      cout << F("Unknown\n");
  }
}
#endif
