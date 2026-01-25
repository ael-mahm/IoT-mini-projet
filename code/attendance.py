import serial
import os
import requests
from datetime import datetime

PORT = 'COM21'     
# Vitesse de communication série
BAUD = 9600
FILE_NAME = "attendance.csv"
BLYNK_TOKEN = "m8Oc17J9iFmHFR8PI9VoH3tg19zIzjSB"
BLYNK_PIN = "V3"

# Cette fonction crée le fichier CSV s'il n'existe pas
# pour enregistrer les données de présence.
def init_csv():
    if not os.path.exists(FILE_NAME):
        with open(FILE_NAME, "w", encoding="utf-8") as f:
            f.write("Name,Status,Date,Time\n")

# Cette fonction envoie les données de présence vers Blynk IoT
# en utilisant une requête HTTP.
def send_to_blynk(value):
    try:
        url = f"https://blynk.cloud/external/api/update?token={BLYNK_TOKEN}&{BLYNK_PIN}={value}"
        requests.get(url, timeout=5)
    except:
        print("⚠ Blynk")

# Cette fonction vérifie si l'étudiant est déjà enregistré aujourd'hui.
def already_marked_today(name, today):
    if not os.path.exists(FILE_NAME):
        return False

    with open(FILE_NAME, "r", encoding="utf-8") as f:
        lines = f.readlines()

    for line in lines[1:]:
        parts = line.strip().split(",")
        if len(parts) >= 4:
            if parts[0] == name and parts[2] == today:
                return True
    return False


# Cette fonction est le programme principal :
# - se connecte à Arduino
# - lit les données série
# - enregistre la présence dans un fichier CSV
# - envoie les données vers Blynk
def main():
    print("Connecting to Arduino...")

    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
    except:
        print("❌ Impossible de se connecter au port.", PORT)
        return

    print("✅ Connecté à", ser.name)

    init_csv()

    print("📡 En attente des données de présence...\n")

    while True:
        if ser.in_waiting:
            raw = ser.readline().decode(errors="ignore").strip()

            if "," not in raw:
                continue

            name, status = raw.split(",")

            now = datetime.now()
            date_str = now.strftime("%Y-%m-%d")
            time_str = now.strftime("%H:%M:%S")

            if already_marked_today(name, date_str):
                print(f"⚠️ {name} Déjà enregistré aujourd’hui.")
                continue

            with open(FILE_NAME, "a", encoding="utf-8") as f:
                f.write(f"{name},{status},{date_str},{time_str}\n")

            # send to Blynk
            message = f"{name} - {status} - {time_str}\n"
            send_to_blynk(message)

            print(f"✅ Présence enregistrée: {name} | {date_str} {time_str}")

if __name__ == "__main__":
    main()
