#include <stdint.h>
#include <string.h>

#include <microsim/MSNet.h>
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
  // printf("enclaveSetup: Position %d\n", position);
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

  // set our initial speed bounds -- normally this would also be negotiated as
  // part of the join procedure
  // The plexe scenarios all start at 100km/h, so we're going to hardcode that
  // in the MSCF_Enclave.h header with an allowed variance of 1 m/s
  status = setInitialSpeedBounds(enclave_id, INITIAL_SPEED_BOUND_LOWER,
                                 INITIAL_SPEED_BOUND_UPPER);
  if (status != CP_SUCCESS) {
    throw ProcessError("Could not set initial enclave speed bounds.");
  }

  // set the initial recovery phase timeout
  long long int t = (long long int)MSNet::getInstance()->getCurrentTimeStep();
  long long int timeout =
      t * STEP_MULTIPLIER + RECOVERY_PHASE_TIMEOUT; // plexe step is .01
                                                    // seconds, so multiply
                                                    // steps by 10 to get time
  status = setInitialRecoveryPhaseTimeout(enclave_id, timeout);
  if (status != CP_SUCCESS) {
    throw ProcessError("Could not set initial Recovery Phase timeout.");
  }
}

bool Enclave::checkIfAllowedSpeed(double speed) {
  bool verdict;
  checkAllowedSpeed(enclave_id, speed, &verdict);
  return verdict;
}

void Enclave::newContractChainGetSignature(
    contract_chain_t contract, cp_ec256_signature_t *return_signature,
    uint8_t num_signatures, cp_ec256_signature_t *signatures) {
  newContractChainGetSignatureCommpact(enclave_id, contract, return_signature,
                                       num_signatures, signatures);
}

uint64_t Enclave::getEnclaveId() const { return enclave_id; }
uint64_t Enclave::setEnclaveId(uint64_t id) { enclave_id = id; }
