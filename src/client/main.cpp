//
// Created by alaa-hassan on 23‏/11‏/2025.
//

#include "client.h"
int main() {
  VechileGatewayClient client("localhost:50051");
  client.Login("VIN123", "T1000");
  client.SendEta("VIN123", "T1000", 15.3, 50.0);
  client.SendStatus("VIN123", "T1000", "moving");
  client.SendArrive("VIN123", "T1000", 17.9, 12.4);
}