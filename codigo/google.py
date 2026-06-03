import requests

# URL do Cataloguer exposta no seu localhost pelo docker-compose2.yml
CATALOG_URL = 'http://localhost:23000'
UUID = 'f47ac10b-58cc-4372-a567-0e02b2c3d474'

def burlar_google_e_registrar():
    print("==================================================")
    print(f"🚀 INICIANDO BYPASS DO GOOGLE MAPS NO INTERSCITY")
    print("==================================================\n")

    print("1. Criando a capacidade 'niveldagua'...")
    cap_payload = {
        "name": "niveldagua",
        "description": "Medidor de nivel de agua",
        "capability_type": "sensor"
    }
    
    try:
        res_cap = requests.post(f"{CATALOG_URL}/capabilities", json=cap_payload)
        if res_cap.status_code in [200, 201]:
            print("   ✅ Capacidade criada com sucesso!")
        elif res_cap.status_code == 400:
            print("   ✅ Capacidade já existia no banco (Perfeito!).")
        else:
            print(f"   ❌ Erro ao criar capacidade: {res_cap.text}")
    except Exception as e:
        print(f"   ❌ Erro de conexão com o Cataloguer: {e}")
        return

    print("\n2. Registrando o Sensor (Injetando cidade/estado para pular o Google)...")
    sensor_payload = {
        "data": {
            "uuid": UUID,
            "description": "Sensor Reservatorio UFMA",
            "status": "active",
            "lat": -2.5307,
            "lon": -44.3068,
            "city": "Sao Luis",        # <-- O SEGREDO ESTÁ AQUI
            "state": "MA",             # <-- E AQUI
            "country": "Brazil",       # <-- E AQUI
            "capabilities": ["niveldagua"]
        }
    }
    
    headers = {'Content-Type': 'application/json'}
    res_sensor = requests.post(f"{CATALOG_URL}/resources", json=sensor_payload, headers=headers)
    
    if res_sensor.status_code in [200, 201]:
        print(f"\n   🎉 SUCESSO ABSOLUTO! O InterSCity aceitou o UUID {UUID} sem chamar o Google.")
        print("   O seu Simulador ESP32 já deve começar a retornar HTTP 201!")
    elif res_sensor.status_code == 422:
        print(f"\n   ⚠️ Retornou 422: O sensor provavelmente JÁ FOI REGISTRADO com este UUID antes.")
        print("   Se o simulador já estiver rodando, verifique se ele começou a dar 201.")
    else:
        print(f"\n   ❌ ERRO: O InterSCity recusou. Código {res_sensor.status_code} | Resposta: {res_sensor.text}")

if __name__ == "__main__":
    burlar_google_e_registrar()