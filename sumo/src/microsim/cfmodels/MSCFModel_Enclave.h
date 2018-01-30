#ifndef MSCFMODEL_ENCLAVE_H
#define MSCFMODEL_ENCLAVE_H

#include <stdint.h>

#include <microsim/cfmodels/MSCFModel.h>

#include <commpact.h>
#include <commpact_types.h>

#define INITIAL_SPEED_BOUND_LOWER ((100.0 / 3.6) - 1.0)
#define INITIAL_SPEED_BOUND_UPPER ((100.0 / 3.6) + 1.0)

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

private:
  uint64_t enclave_id;
  cp_ec256_public_t pubkey;
};

#endif // MSCFMODEL_ENCLAVE_H
