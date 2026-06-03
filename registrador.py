import requests
import json

CATALOG_URL = 'http://localhost:23000'
# Novo UUID que definimos
UUID = 'f47ac10b-58cc-4372-a567-0e02b2c3d474'

def executar_setup():
    print(f"🚀 Iniciando setup para o sensor: {UUID}")

    # 1. Tentar registrar a Capability
    # Se der erro 400 (Bad Request), é porque já existe, o que é ótimo.
    cap_payload = {
        "name": "niveldagua",
        "description": "Medidor de nivel de agua",
        "capability_type": "sensor"
    }
    r_cap = requests.post(f'{CATALOG_URL}/capabilities', json=cap_payload)
    print(f"Status Capability: {r_cap.status_code}")

    # 2. Tentar registrar o Recurso
    # Estrutura com 'data' como raiz, pois vimos no log que o Rails exige isso.
    res_url = f'{CATALOG_URL}/resources'
    payload = {
        "data": {
            "uuid": UUID,
            "description": "Sensor Reservatorio UFMA",
            "status": "active",
            "lat": -2.5307,
            "lon": -44.3068,
            "capabilities": ["niveldagua"] 
        }
    }
    
    headers = {'Content-Type': 'application/json'}
    r_res = requests.post(res_url, data=json.dumps(payload), headers=headers)
    
    if r_res.status_code == 201:
        print("✅ Recurso criado com sucesso e associado!")
    elif r_res.status_code == 422:
        print("⚠️ Atenção: O catálogo recusou o campo 'capabilities'.")
        print("Tentando criar o recurso básico sem a associação automática...")
        
        # Tenta criar apenas o básico
        payload["data"].pop("capabilities")
        r_res_simples = requests.post(res_url, data=json.dumps(payload), headers=headers)
        print(f"Status da tentativa de criação básica: {r_res_simples.status_code}")
        print(f"Resposta: {r_res_simples.text}")
    else:
        print(f"❌ Erro ao criar recurso: {r_res.status_code}")
        print(r_res.text)

if __name__ == "__main__":
    executar_setup()