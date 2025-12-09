# tietoliikenneprojekti_r4
# README

## Tietoliikenteen sovellusprojekti  
**OAMK Tieto- ja viestintätekniikka – 2. vuoden syksy**

**@harkonennen** – Harri Härkönen  
**@JoonasHeiskanen** – Joonas Heiskanen  

---

## Projektin kuvaus

Projektin keskipisteenä on datan mittaaminen, siirtäminen ja käsittely eri teknologioilla. Kiihtyvyyssensoria mitataan nRF5340-alustalla, josta data siirretään BLE-yhteyttä  
hyödyntäen ensin Raspberry Pille ja siitä TCP-yhteydellä Linux-palvelimelle. Dataa haetaan MySQL-tietokannasta HTTP-rajapinnalla ja syötetään Python-algoritmille päättelemään sensorin orientaatio.

*(tähän tulee kuva)*

---

## Arkkitehtuurikuva

---

## Työkalut

Projekti on tehty hyödyntäen eri työkaluja, rautaa ja ympäristöjä. Alta löytyy lista työkaluista, joita projektissa on käytetty:

- **Editori:** Visual Studio Code  
- **Koodikielet:** Python, C  
- **Mikrokontrolleri:** Nordic Semiconductor nRF5340DK  
- **Datansiirto:** Bluetooth Low Energy (BLE)  
- **Kiihtyvyysanturi:** GY-61 ADXL335 3-Axis Accelerometer Module  
- **Kehitysalusta:** Raspberry Pi  
- **Tietokanta:** MySQL  
- **Palvelin:** Ubuntu Linux  
- **Versionhallinta:** GitHub, GitBash  

---

## Datan mittaaminen

Kiihtyvyyssensorista mitataan dataa NRF5340 alustalla. 3-akselinen kiihtyvyysensori mittaa dataa X, Y, Z-akseleilta. Mittaaminen tapahtuu painamalla nappia, jolloin haluttu määrä näytteitä otetaan talteen ja lähetetään. Oletuksena näytteenottotaajuus on 100 näytettä sekunnissa.

---

## Datan käsittely

NRF5340 lähettää Bluetooth Low Energy -yhteyden yli datapaketteja Raspberry 3:lle.. Raspberryllä pyörivä python-koodi pakkaa datan ja lähettää sen TCP-rajapinnan yli Linux-palvelimelle MySQL-tietokantaan, johon data tallennetaan.

---

## Datan käyttö

Data haetaan HTTP-yhteyden yli MySQL-tietokannasta ja tallennetaan tietokoneella CSV-tiedostoon. Python-koodi lukee tämän CSV-tiedoston ja vie tämän datan K-means algoritmin läpi. Dataset kuva osiossa mallinnetaan CSV-tiedostosta luetut datapisteet, jonka jälkeen arvotaan satunnaiset sentroidit toisessa kuvassa. Viimeisessä kuvassa python-koodi on toteutettu siten, että lähimmäisenä datapisteitä olevat sentroidit voittavat lähimmäiset datapisteet itselleen ja siirtyy kyseisten datapisteiden keskelle. Tuloksena saadaan tieto, mihin päin sensori osoittaa tai mikä on sen suunta.

Kuva, joka sisältää kohteen diagrammi, teksti, Suunnitelma, viiva  
*Tekoälyllä luotu sisältö voi olla virheellistä.*  
K-means 3D datapistekuvat eri vaiheista.  
*(tähän tulee kuva)*

Siniset pallot kuvastavat datapisteitä ja punaiset tähdet sentroideja.

---

K-means-algoritmin laskemat kuusi keskipistettä siirretään nRF5340-laitteelle. Laitteen kiihtyvyysanturilla mitataan 100 näytettä kuuteen eri suuntaan, ja jokainen mittaus luokitellaan valitsemalla lähin keskipiste euklidisen etäisyyden perusteella. Luokittelun tuloksista lasketaan confusion matrix, joka kuvaa luokittelijan tarkkuutta.  
*(tähän tulee kuva)*  
Esimerkkituloksena saatu confusion matrix, jossa K-means-luokittelija tunnistaa kaikki suunnat oikein.
