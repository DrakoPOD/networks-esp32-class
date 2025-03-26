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
    # simple tls context
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(certfile='certs/server_cert.pem', keyfile='certs/server_key.pem')
    app.run(ssl_context=context, host='0.0.0.0', port=443)
