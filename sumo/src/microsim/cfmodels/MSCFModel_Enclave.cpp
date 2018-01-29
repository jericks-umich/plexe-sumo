#include <stdint.h>
#include <string.h>

#include <utils/common/UtilExceptions.h>

#include "MSCFModel_Enclave.h"

#include <commpact.h>
#include <commpact_types.h>

Enclave::Enclave() : enclave_id(0) {
  memset(&pubkey.gx, 0, CP_NISTP_ECP256_KEY_SIZE);
  memset(&pubkey.gy, 0, CP_NISTP_ECP256_KEY_SIZE);
  commpact_status_t status;
  status = initEnclave(&enclave_id);
  if (status != CP_SUCCESS) {
    throw ProcessError("Could not initialize enclave.");
  }
}

void Enclave::enclaveVehicleSetup(int position) {
  commpact_status_t status;
  printf("enclaveSetup: Position %d\n", position);
  // inform the enclave of this vehicle's position
  status = setInitialPosition(enclave_id, position);
  if (status != CP_SUCCESS) {
    throw ProcessError("Could not set enclave position..");
  }
  // initialize enclave keypair and set pubkey
  // this must currently be done after setting the position index for the
  // enclave because we are using the InitialSetup singleton class hack
  status = initializeKeys(enclave_id, &pubkey);
  if (status != CP_SUCCESS) {
    throw ProcessError("Could not initialize enclave key pair.");
  }
}

bool Enclave::checkIfAllowedSpeed(double speed) {
  bool verdict;
  checkAllowedSpeed(enclave_id, speed, &verdict);
  return verdict;
}

uint64_t Enclave::getEnclaveId() const { return enclave_id; }
uint64_t Enclave::setEnclaveId(uint64_t id) { enclave_id = id; }
