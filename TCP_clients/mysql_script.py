import mysql.connector
from dotenv import load_dotenv
import os

# Lataa ympäristömuuttujat .env-tiedostosta
load_dotenv() 

# ====================================================================
# ASETUKSET
# ====================================================================

DB_CONFIG = {
    # Hakee tiedot .env-tiedostosta
    'host': os.getenv('DB_HOST'), 
    'user': os.getenv('DB_USER'),
    'password': os.getenv('DB_PASS'),
    'database': os.getenv('DB_NAME'),
}

# SQL-kysely: Hae 10 uusinta tietuetta rawdata-taulusta
# Muista, että 'timestamp' on tietokannassasi
SQL_SELECT = "SELECT * FROM rawdata"

# ====================================================================

def fetch_data_from_database():
    """
    Muodostaa yhteyden MySQL-tietokantaan, suorittaa SELECT-kyselyn
    ja tulostaa haetun datan.
    """
    conn = None
    try:
        # 1. Yhdistä tietokantaan
        print(f"Yritetään yhdistää tietokantaan '{DB_CONFIG['database']}' osoitteessa {DB_CONFIG['host']}...")
        conn = mysql.connector.connect(**DB_CONFIG)

        if not conn.is_connected():
            print("VIRHE: Tietokantayhteys epäonnistui.")
            return

        cursor = conn.cursor()
        
        # 2. Suorita SELECT-kysely
        cursor.execute(SQL_SELECT)

        # 3. Hae kaikki tulokset (fetch all)
        # Tulokset ovat tuplen (tuple) listana.
        results = cursor.fetchall()

        # 4. Tulosta tulokset
        print("-" * 50)
        print(f"Haettu {len(results)} tietuetta:")
        print("-" * 50)
        
        # Tulostetaan sarakkeiden nimet (valinnaisesti)
        column_names = [i[0] for i in cursor.description]
        print(f"| {' | '.join(column_names)} |")
        print("-" * 50)

        for row in results:
            # Muunna jokainen arvo merkkijonoksi tulostusta varten
            row_str = [str(item) for item in row]
            print(f"| {' | '.join(row_str)} |")

        print("-" * 50)

    except mysql.connector.Error as err:
        print(f"VIRHE: Tietokantavirhe: {err}")
        # Voit lisätä tarkemman käsittelyn tunnistautumisvirheille (esim. 1045)
    except Exception as e:
        print(f"Yleinen virhe: {e}")
        
    finally:
        # 5. Sulje yhteys (aina tärkeää)
        if conn and conn.is_connected():
            cursor.close()
            conn.close()
            print("Yhteys suljettu.")


if __name__ == "__main__":
    fetch_data_from_database()