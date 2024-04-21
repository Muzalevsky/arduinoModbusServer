
from pymodbus.client import ModbusTcpClient

if __name__ == "__main__":
  c = ModbusTcpClient("192.168.13.2")
  UNIT = 0x1
  c.write_coil(1,True,unit=UNIT) # Turns LED on
  c.write_coil(2,True,unit=UNIT)

  r = c.read_coils(1,1,unit=UNIT)
  print(r, type(r), r.bits)  # Shows status of LED
  
  c.write_register(0,2, unit=UNIT)


