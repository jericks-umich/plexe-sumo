#ifndef MSCFMODEL_ENCLAVE_H
#define MSCFMODEL_ENCLAVE_H

#include <stdint.h>

#include <microsim/cfmodels/MSCFModel.h>

#include <commpact.h>
#include <commpact_types.h>

#define INITIAL_SPEED_BOUND_LOWER 26.777000 // ((100.0 / 3.6) - 1.0)
#define INITIAL_SPEED_BOUND_UPPER 28.777000 // ((100.0 / 3.6) + 1.0)

#define RECOVERY_PHASE_TIMEOUT 500 // milliseconds
#define STEP_MULTIPLIER 10         // plexe step is in .01 seconds, so multiply
                                   // steps by 10 to get time

class Enclave {
public:
  Enclave();
  void enclaveVehicleSetup(int position);
  uint64_t getEnclaveId() const;
  uint64_t setEnclaveId(uint64_t id);
  bool checkIfAllowedSpeed(double speed);
  void newContractChainGetSignature(contract_chain_t contract,
                                    cp_ec256_signature_t *return_signature,
                                    uint8_t num_signatures,
                                    cp_ec256_signature_t *signatures);

private:
  uint64_t enclave_id;
  cp_ec256_public_t pubkey;
};

#endif // MSCFMODEL_ENCLAVE_H
