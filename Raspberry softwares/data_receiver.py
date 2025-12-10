import asyncio
import struct
import mysql.connector
from bleak import BleakClient, BleakScanner
import sys
import os
from dotenv import load_dotenv

load_dotenv()

# 1. BLE-DEVICE CONFIQS (NRF5340)
# NRF5340:n MAC-address/UUID
DEVICE_ADDRESS = "C7:2E:D6:BF:B5:B5" 
# MYSENSOR Characteristic UUID, where NRF5340 sends data
CHAR_UUID = "00001526-1212-efde-1523-785feabcd123"

# HHHH = 4 x uint16_t (dir, X, Y, Z - 2 byte each)
STRUCT_FORMAT = '<HHHH' 
EXPECTED_LENGTH = struct.calcsize(STRUCT_FORMAT)

# 2. MYSQL-DATABASE INFOS
DB_CONFIG = {
    'host': os.getenv('DB_HOST'),
    'user': os.getenv('DB_USER'),
    'password': os.getenv('DB_PASS'),
    'database': os.getenv('DB_NAME')
}
# SQL-data saving
SQL_INSERT = "INSERT INTO rawdata (sensor_direction, sensorvalue_x, sensorvalue_y, sensorvalue_z) VALUES (%s, %s, %s, %s)"


# Global database connetion
db_connection = None

def get_db_connection():
    """Connecting MySQL-database."""
    global db_connection
    try:
        if db_connection is None or not db_connection.is_connected():
            db_connection = mysql.connector.connect(**DB_CONFIG)
            print("INFO: MySQL-connection made succesfull.")
        return db_connection
    except mysql.connector.Error as err:
        print(f"ERROR: MySQL-connection failed: {err}")
        db_connection = None
        return None

def notification_handler(sender: int, data: bytearray):
    """
    Handling nRF5340 BLE-notification.
    Extract 8 byte packet (Direction, X, Y, Z) and prints it.
    """

    conn = get_db_connection()
    if conn is None:
        print("DB-connection missing. No data saving.")
        return
    
    # Checks the data length
    if len(data) != EXPECTED_LENGTH:
        print(f"ERROR: Rereived data length ({len(data)} bytes) does not fit ({EXPECTED_LENGTH} bytes). Skipping data-packet.", file=sys.stderr)
        return

    try:
        # Extracting data: (uint16_t direction, uint16_t x, uint16_t y, uint16_t z)
        direction, x_val, y_val, z_val = struct.unpack(STRUCT_FORMAT, data)
        
        # Adding values to database
        cursor = conn.cursor()
        data_to_save = (direction, x_val, y_val, z_val)
        cursor.execute(SQL_INSERT, data_to_save)
        conn.commit()
        cursor.close()
                

        print(f"Data saved: Dir={direction}, X={x_val}, Y={y_val}, Z={z_val}")
        

        # Prints the extract values
        print(f"[{asyncio.get_event_loop().time():.2f}] Datavalues: ", end="")
        print(f"DIR: {direction}, X: {x_val}, Y: {y_val}, Z: {z_val}")

    except struct.error as e:
        print(f"ERROR: Data extracting failed: {e}", file=sys.stderr)

    except mysql.connector.Error as err:
        print(f"ERROR: DB-operaation failed: {err}")
        # Trying to close conneciton if something went wrong
        if conn and conn.is_connected():
             conn.close()


async def run_ble_client(address: str, char_uuid: str):
    """Connection to BLE-Device and starts to listening data"""
    
    print(f"Trying to connect device address {address}...")
    
    while True: # Endless loop to try reconnect
        try:
            async with BleakClient(address, timeout=10.0) as client:
                if not client.is_connected:
                    print("WARNING: Connection failed (BleakClient not connected).")
                    await asyncio.sleep(5)
                    continue

                print("INFO: Connection succesfull! Ordering notifications.")
                
                # Order notifications
                await client.start_notify(char_uuid, notification_handler)

                print("Listening data... (Press CTRL+C to end)")
                # Client is open and listen to notifications
                while client.is_connected:
                    await asyncio.sleep(1)
                
                print("WARNING: BLE-connection disconnected. Trying to reconnect in 5 seconds...")
                await asyncio.sleep(5)

        except Exception as e:
            # Other errors (timeout, Bleak-errors etc)
            print(f"CRITICAL: Unexpected error in Bleak-connection: {e}. Trying to reconnect again after 10 seconds...", file=sys.stderr)
            await asyncio.sleep(10)


async def main():
    """Starts async main loop."""
    if get_db_connection() is None:
        print("CRITICAL: Invalid database settings or db is not reachable. Check settings.")
        return

    await run_ble_client(DEVICE_ADDRESS, CHAR_UUID)


if __name__ == "__main__":
    try:
        # Run main async
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nSoftware shutdowned by user (CTRL+C).")
    except Exception as e:
        print(f"FATAL ERROR: {e}", file=sys.stderr)
    finally:
        # Closing database connection
        if db_connection and db_connection.is_connected():
            db_connection.close()
            print("INFO: MySQL-connection closed.")