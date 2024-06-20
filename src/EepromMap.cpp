#include <Arduino.h>
#include <EEPROM.h>
#include "EepromMap.h"

/*--[ Literals ]-----------------------------------------------------------------------------------------------------------------*/
#define KANARI_VALUE  (0x18)

enum EEPROM_MAP
{
  KANARIE = 0,
  TAFEL_HOMED,
  GEKOSE_POSISIE_INDEX,
  TAFEL_POSISIE,
  POSISIES_BEGIN = 0x20
};

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void EepromMapInitialise()
{
  uint8_t kanari = EEPROM.read(KANARIE);
  if (kanari != KANARI_VALUE)
  {
    EEPROM.write(KANARIE, KANARI_VALUE);
    EEPROM.write(TAFEL_HOMED, !HOMED);
    EEPROM.write(GEKOSE_POSISIE_INDEX, 0);

    EEPROM.put(TAFEL_POSISIE, 0);
    
    // Make all positions the same
    for (uint8_t i = 0; i < MAX_POSISIES; i += sizeof(uint32_t))
    {
      EEPROM.put(POSISIES_BEGIN + i, 0);
    }
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
uint8_t IsTafelHomed()
{
  uint8_t e = EEPROM.read(TAFEL_HOMED);
  return e;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void TafelIsHomed()
{
  EEPROM.update(TAFEL_HOMED, HOMED);
  return;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
uint32_t KryTafelPosisie()
{
  uint32_t e;
  EEPROM.get(TAFEL_POSISIE, e);
  return e;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
uint8_t KryGekosePosisieIndex()
{
  uint8_t e = EEPROM.read(GEKOSE_POSISIE_INDEX);
  return e;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
uint32_t KryGeStoordePosisie(uint8_t posisieIndex)
{
  uint32_t e;
  EEPROM.get(POSISIES_BEGIN + (posisieIndex * sizeof(uint32_t)), e);
  return e;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void StoorPosisie(uint8_t posisieIndex, uint32_t posisie)
{
  EEPROM.put(POSISIES_BEGIN + (posisieIndex * sizeof(uint32_t)), posisie);
  return;
}