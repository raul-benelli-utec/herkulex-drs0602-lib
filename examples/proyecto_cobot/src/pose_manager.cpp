#include "pose_manager.h"

PoseManager::PoseManager() : nextSlot(0) {
  // Inicializar todas las poses como no usadas
  for (uint8_t i = 0; i < MAX_POSES; i++) {
    userPoses[i].used = false;
    userPoses[i].name[0] = '\0';
    for (uint8_t j = 0; j < NUM_MOTORES; j++) {
      userPoses[i].pos[j] = 0;
    }
  }
}

bool PoseManager::isValidSlot(uint8_t slotIndex) const {
  return (slotIndex >= 1 && slotIndex <= MAX_POSES);
}

uint8_t PoseManager::slotToIndex(uint8_t slotIndex) const {
  return slotIndex - 1;  // Convertir 1..20 a 0..19
}

uint8_t PoseManager::indexToSlot(uint8_t index) const {
  return index + 1;  // Convertir 0..19 a 1..20
}

void PoseManager::generateAutoName(uint8_t slotIndex, char* outName) const {
  outName[0] = 'P';
  if (slotIndex < 10) {
    outName[1] = '0';
    outName[2] = '0' + slotIndex;
    outName[3] = '\0';
  } else {
    outName[1] = '0' + (slotIndex / 10);
    outName[2] = '0' + (slotIndex % 10);
    outName[3] = '\0';
  }
}

bool PoseManager::capturePoseSlot(uint8_t slotIndex, const char* nameOpt, uint16_t* currentPositions) {
  if (!isValidSlot(slotIndex)) {
    return false;
  }
  
  uint8_t idx = slotToIndex(slotIndex);
  
  // Verificar que todas las posiciones sean válidas
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    if (currentPositions[i] == 0xFFFF) {
      return false;  // Error al leer posición
    }
  }
  
  // Guardar posiciones
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    userPoses[idx].pos[i] = currentPositions[i];
  }
  
  // Guardar nombre
  if (nameOpt != nullptr && nameOpt[0] != '\0') {
    strncpy(userPoses[idx].name, nameOpt, 11);
    userPoses[idx].name[11] = '\0';  // Asegurar null terminator
  } else {
    generateAutoName(slotIndex, userPoses[idx].name);
  }
  
  userPoses[idx].used = true;
  return true;
}

uint8_t PoseManager::capturePoseAuto(const char* nameOpt, uint16_t* currentPositions) {
  uint8_t slot = nextSlot + 1;  // Convertir índice a slot (1..20)
  
  if (capturePoseSlot(slot, nameOpt, currentPositions)) {
    nextSlot = (nextSlot + 1) % MAX_POSES;  // Circular 0..19
    return slot;
  }
  
  return 0;  // Falló
}

bool PoseManager::playPoseSlot(uint8_t slotIndex, uint16_t* outPositions) const {
  if (!isValidSlot(slotIndex)) {
    return false;
  }
  
  uint8_t idx = slotToIndex(slotIndex);
  
  if (!userPoses[idx].used) {
    return false;
  }
  
  // Copiar posiciones al array de salida
  for (uint8_t i = 0; i < NUM_MOTORES; i++) {
    outPositions[i] = userPoses[idx].pos[i];
  }
  
  return true;
}

void PoseManager::listUserPoses(Stream& out) const {
  out.println(F("=== Poses del usuario ==="));
  out.println();
  
  bool found = false;
  for (uint8_t i = 0; i < MAX_POSES; i++) {
    if (userPoses[i].used) {
      found = true;
      uint8_t slot = indexToSlot(i);
      out.print(F("Slot "));
      if (slot < 10) out.print(F("0"));
      out.print(slot);
      out.print(F(": "));
      out.print(userPoses[i].name);
      out.print(F(" | Pos: "));
      
      for (uint8_t j = 0; j < NUM_MOTORES; j++) {
        // Formatear con 5 dígitos
        uint16_t pos = userPoses[i].pos[j];
        if (pos < 10000) out.print(F("0"));
        if (pos < 1000) out.print(F("0"));
        if (pos < 100) out.print(F("0"));
        if (pos < 10) out.print(F("0"));
        out.print(pos);
        if (j < NUM_MOTORES - 1) out.print(F(" "));
      }
      out.println();
    }
  }
  
  if (!found) {
    out.println(F("(No hay poses guardadas)"));
  }
  
  out.println();
}

bool PoseManager::deletePoseSlot(uint8_t slotIndex) {
  if (!isValidSlot(slotIndex)) {
    return false;
  }
  
  uint8_t idx = slotToIndex(slotIndex);
  
  if (!userPoses[idx].used) {
    return false;  // Ya estaba vacío
  }
  
  userPoses[idx].used = false;
  userPoses[idx].name[0] = '\0';
  
  return true;
}

bool PoseManager::isSlotUsed(uint8_t slotIndex) const {
  if (!isValidSlot(slotIndex)) {
    return false;
  }
  return userPoses[slotToIndex(slotIndex)].used;
}

const char* PoseManager::getPoseName(uint8_t slotIndex) const {
  if (!isValidSlot(slotIndex)) {
    return nullptr;
  }
  uint8_t idx = slotToIndex(slotIndex);
  if (!userPoses[idx].used) {
    return nullptr;
  }
  return userPoses[idx].name;
}

uint8_t PoseManager::getUsedSlots(uint8_t* outSlots, uint8_t maxSlots) const {
  uint8_t count = 0;
  for (uint8_t i = 0; i < MAX_POSES && count < maxSlots; i++) {
    if (userPoses[i].used) {
      outSlots[count++] = indexToSlot(i);
    }
  }
  return count;
}

