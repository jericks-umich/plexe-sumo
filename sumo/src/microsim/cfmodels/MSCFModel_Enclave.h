#ifndef MSCFMODEL_ENCLAVE_H
#define MSCFMODEL_ENCLAVE_H

#include <stdint.h>

#include <microsim/cfmodels/MSCFModel.h>

#include <commpact.h>
#include <commpact_types.h>

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
