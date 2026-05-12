from flask import Flask, jsonify

app = Flask(__name__)

@app.route('/interscity_lh/collector/resources/<uuid>/data/last', methods=['GET'])
def get_last_data(uuid):
    # Aqui você altera o valor para testar o Bot
    # Se colocar abaixo de 15, o WhatsApp deve apitar
    return jsonify({
        "data": {
            "niveldagua": 22.5 
        }
    })

if __name__ == '__main__':
    # Roda na porta 5000
    app.run(host='0.0.0.0', port=5000)