from flask import Flask, jsonify
import ssl

app = Flask(__name__)

@app.route("/version")
def version():
	return jsonify({"version": "1.0.0"})
	
@app.route("/bin")
def bin_data():
	return "Bin file"

if __name__ == "__main__":
	# Usar los certificados para HTTPS
	context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
	context.load_cert_chain(certfile='certs/server.crt', keyfile='certs/server.key')
	context.load_verify_locations(cafile='certs/ca.crt') # Certificado de la CA que firmó los clientes
	context.verify_mode = ssl.CERT_REQUIRED # ⚠️ Requerir certificados de cliente
	
	app.run(ssl_context=context, host='0.0.0.0', port=443)