from flask import Flask, jsonify

app = Flask(__name__)

@app.route("/version")
def version():
    return jsonify({"version": "1.0.0"})

@app.route("/bin")
def bin_data():
    return "Bin file"

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=80)