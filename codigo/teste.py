import requests

CATALOG_URL = 'http://interscity-resource-cataloguer:3000' 
UUID = 'f47ac10b-58cc-4372-a567-0e02b2c3d474'

def setup_total():
    print("1. Criando a capacidade 'niveldagua'...")
    try:
        res_cap = requests.post(
            f'{CATALOG_URL}/capabilities', 
            json={"name": "niveldagua", "description": "Medidor", "capability_type": "sensor"}
        )
        print(f"Resultado da Capacidade: {res_cap.status_code} - {res_cap.text}")
    except Exception as e:
        print(f"Erro de conexão: {e}")
        return
    
    print("\n2. Registrando o Sensor (Tentando forçar todos os campos geográficos)...")
    payload = {
        "data": {
            "uuid": UUID,
            "description": "Sensor Reservatorio UFMA",
            "status": "active",
            "lat": -2.5307, 
            "lon": -44.3068,
            "city": "Sao Luis",
            "state": "MA",
            "country": "Brazil",
            "neighborhood": "Itaqui-Bacanga",
            "postal_code": "65080-805",
            "capabilities": ["niveldagua"]
        }
    }
    
    try:
        r = requests.post(
            f'{CATALOG_URL}/resources', 
            json=payload, 
            headers={'Content-Type': 'application/json'}
        )
        print(f"Resultado do Cadastro: {r.status_code} - {r.text}")
        
    except Exception as e:
        print(f"Erro ao tentar registrar o recurso: {e}")

if __name__ == "__main__":
    setup_total()