#ifndef POSE_MANAGER_H
#define POSE_MANAGER_H

#include <Arduino.h>
#include "brazo_config.h"

// Máximo número de poses del usuario
#define MAX_POSES 20

// Estructura para almacenar una pose del usuario
struct PoseSlot {
  bool used;
  char name[12];              // Nombre de la pose (máximo 11 caracteres + null)
  uint16_t pos[NUM_MOTORES];   // Posiciones de los motores
};

// Gestor de poses del usuario
class PoseManager {
public:
  PoseManager();
  
  // Capturar pose actual en un slot específico (1..20)
  // Retorna: true si se capturó correctamente
  bool capturePoseSlot(uint8_t slotIndex, const char* nameOpt, uint16_t* currentPositions);
  
  // Capturar pose en el siguiente slot disponible (autoincremental)
  // Retorna: número de slot usado (1..20) o 0 si falló
  uint8_t capturePoseAuto(const char* nameOpt, uint16_t* currentPositions);
  
  // Reproducir pose de un slot (1..20)
  // Retorna: true si se reprodujo correctamente
  bool playPoseSlot(uint8_t slotIndex, uint16_t* outPositions) const;
  
  // Listar todas las poses guardadas
  void listUserPoses(Stream& out) const;
  
  // Eliminar pose de un slot (1..20)
  // Retorna: true si se eliminó correctamente
  bool deletePoseSlot(uint8_t slotIndex);
  
  // Verificar si un slot está usado
  bool isSlotUsed(uint8_t slotIndex) const;
  
  // Obtener el nombre de una pose
  const char* getPoseName(uint8_t slotIndex) const;
  
  // Recorrer todas las poses usadas en orden
  // Retorna: número de poses encontradas
  uint8_t getUsedSlots(uint8_t* outSlots, uint8_t maxSlots) const;

private:
  PoseSlot userPoses[MAX_POSES];
  uint8_t nextSlot;
  
  // Validar índice de slot (1..20 -> 0..19)
  bool isValidSlot(uint8_t slotIndex) const;
  uint8_t slotToIndex(uint8_t slotIndex) const;
  uint8_t indexToSlot(uint8_t index) const;
  
  // Generar nombre automático "P01".."P20"
  void generateAutoName(uint8_t slotIndex, char* outName) const;
};

#endif // POSE_MANAGER_H

