//
// Created by alaa-hassan on 23‏/11‏/2025.
//

#include "client.h"
int main ()
{
    VechileGatewayClient client("localhost:50051");
  //  client.Login("VIN123", "T1000", 31.21, 29.99, 31.30, 30.10);
    //client.SendEta("VIN123", "T1000", 15.3, 50.0);
    client.SendStatus("VIN123", "T1000", "moving");

}