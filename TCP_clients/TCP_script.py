import requests
from bs4 import BeautifulSoup
import csv
import os

# ====================================================================
# ASETUKSET
# ====================================================================

# UUSI ASETUS: RIVIMÄÄRÄ
MAX_ROWS = 100

API_URL = "http://172.20.241.17/measurements.php"
CSV_FILE_NAME = "html_tiedot.csv"

# HUOM: Sarakkeiden otsikot on määriteltävä käsin, koska ne puuttuvat HTML:stä
CSV_HEADERS = ['id', 'timestamp', 'sensor_dir', 'sensorvalue_x', 'sensorvalue_y', 'sensorvalue_z']

# ====================================================================

def fetch_and_save_data_from_html():
    """
    Hakee dataa URL:sta, purkaa sen HTML-muodosta ja tallentaa CSV:hen.
    """
    print(f"1. Yritetään hakea dataa osoitteesta: {API_URL}")
    
    try:
        # Suoritetaan HTTP GET -pyyntö
        response = requests.get(API_URL)
        response.raise_for_status() 
        
        # Käytä .text, koska sisältö on merkkijonoa (HTML)
        html_content = response.text 
        
    except requests.exceptions.RequestException as e:
        print(f"VIRHE: HTTP-pyyntö epäonnistui: {e}")
        return

    # 2. Purkaminen BeautifulSoupilla
    soup = BeautifulSoup(html_content, 'html.parser')
    
    # Etsi kaikki taulukon rivit (oletetaan, että data on <tr>-tagien sisällä)
    # HUOM: Sinun täytyy ehkä muokata hakuehtoa, jos skriptisi sijoittaa 
    # rivit jonkin tietyn <table id="data"> tai <tbody> sisään.
    rows = soup.find_all('tr')
    
    # Kerätään purettu data
    extracted_data = []
    
    # Aloitetaan indeksistä 1, jos taulukon ensimmäinen rivi on otsikko
    start_index = 0 # Jos HTML-vastaus alkaa suoraan datariveillä
    
    # Käy läpi löytyneet rivit, rajoitetaan samalla 10 ensimmäiseen
    for row in rows[start_index:]:
        if len(extracted_data) >= MAX_ROWS:
            break # Saavutettu maksimimäärä
            
        # Etsi kaikki solut (<td>) rivin sisältä
        cells = row.find_all('td')
        
        if len(cells) == len(CSV_HEADERS):
            # Kerää solujen tekstisisältö listaksi
            row_data = [cell.text.strip() for cell in cells]
            extracted_data.append(row_data)

    if not extracted_data:
        print("VAROITUS: HTML-muotoista dataa ei löytynyt (tai rakenne oli väärä). Tarkista URL/PHP-tuloste.")
        print(f"RAAKA ALKU: {html_content[:500]}")
        return

    print(f"2. HTML-purku onnistui. Tallennetaan {len(extracted_data)} tietuetta.")

    # 3. Tallennetaan CSV-tiedostoon
    try:
        with open(CSV_FILE_NAME, 'w', newline='', encoding='utf-8') as csvfile:
            writer = csv.writer(csvfile)
            
            # Kirjoita otsikkorivi
            writer.writerow(CSV_HEADERS)
            
            # Kirjoita puretut datatietueet
            writer.writerows(extracted_data)
            
        print(f"3. Data tallennettu onnistuneesti tiedostoon: {os.path.abspath(CSV_FILE_NAME)}")

    except Exception as e:
        print(f"VIRHE: Datan kirjoittaminen CSV-tiedostoon epäonnistui: {e}")


if __name__ == "__main__":
    fetch_and_save_data_from_html()