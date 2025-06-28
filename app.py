from flask import Flask, render_template, jsonify
from lector_serial import AforoReader

app = Flask(__name__)
lector = AforoReader(puerto="COM3")  #Revisar puerto

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/datos')
def datos():
    return jsonify(lector.obtener_datos())

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
