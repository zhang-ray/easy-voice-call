

### Client <---> Server  workflow
- In general speaking:
  - C->S          TCP/UDP connect
  - C->S          Login Request
  - S->C          Login Response (with token?)
  - C->S          User Info (friendly User name, etc...)
  - C->S / S->C   Audio Packet
  - C->S          Logout Request
  - S->C          Logout Response
- random packet:
  - C->S / S->C   HeartBeatRequest
  - S->C / C->S   HeartBeatResponse