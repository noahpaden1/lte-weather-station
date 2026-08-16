from flask import Flask, request
from supabase import create_client
import os
import json

app = Flask(__name__)

supabase = create_client(
    os.environ["SUPABASE_URL"],
    os.environ["SUPABASE_SERVICE_KEY"]
)

@app.route("/", methods=["GET"])
def home():
    return "IoT Weather Station API Running", 200

@app.route("/sms", methods=["POST"])
def sms_webhook():
    from_number = request.form.get("From")
    body = request.form.get("Body")

    print(f"Incoming SMS From: {from_number}")
    print(f"Raw Payload Body: {body}")

    try:
        data = json.loads(body)

        payload = {
            "phone_number": from_number,
            
            "temperature": data.get("t"),
            "humidity":    data.get("h"),
            "battery":     data.get("v"),    
            "air_pressure": data.get("p"),  
            "wind_speed":   data.get("w"),
            
            "latitude":    data.get("lat"),
            "longitude":   data.get("lng")
        }

        response = supabase.table("sensor_data").insert(payload).execute()
        print("Successfully Inserted:", response)

    except json.JSONDecodeError:
        print("Error: The SMS body was not valid JSON.")
    except Exception as e:
        print("Error parsing/inserting data:", e)

    return """<?xml version="1.0" encoding="UTF-8"?>
<Response></Response>""", 200

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
