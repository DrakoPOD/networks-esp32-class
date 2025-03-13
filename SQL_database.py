import psycopg2

user = "postgres"
password = "12345678"

# Conexión a la base de datos
conn = psycopg2.connect(
    dbname="postgres",  # Conéctate a la base de datos "postgres" o a la que desees
    user=user,    # Usuario
    password=password,  # Contraseña
    host="127.0.0.1",   # Servidor local
    port="5432"         # Puerto por defecto
)

# Crear un cursor
cur = conn.cursor()

# Asegúrate de ejecutar CREATE DATABASE fuera de una transacción
cur.execute("COMMIT;")  # Se asegura de cerrar cualquier transacción activa
cur.execute("DROP DATABASE IF EXISTS test_db;")  # Elimina la base de datos si existe
# Crear base de datos
cur.execute("CREATE DATABASE test_db;")

# Cerrar la conexión a la base de datos principal
cur.close()
conn.close()

# Conectarse a la nueva base de datos
conn = psycopg2.connect(
    dbname="test_db",  # Conéctate a la base de datos que creaste
    user=user,
    password=password,
    host="127.0.0.1",
    port="5432"
)

cur = conn.cursor()

# Crear tabla
cur.execute("""
CREATE TABLE personas (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100),
    edad INT
);
""")

# Insertar datos en la tabla
cur.execute("INSERT INTO personas (nombre, edad) VALUES (%s, %s)", ("Juan", 30))
cur.execute("INSERT INTO personas (nombre, edad) VALUES (%s, %s)", ("Ana", 26.6))

# Guardar los cambios y cerrar la conexión
conn.commit()
cur.close()
conn.close()

print("Base de datos y datos creados correctamente.")

# Conexión a la base de datos
conn = psycopg2.connect(
    dbname="test_db",  # Asegúrate de que la base de datos "test_db" exista
    user=user,
    password=password,
    host="127.0.0.1",
    port="5432"
)

# Crear un cursor
cur = conn.cursor()

# Leer datos
cur.execute("SELECT * FROM personas;")

# Obtener todos los resultados
personas = cur.fetchall()

# Imprimir resultados
for persona in personas:
    print(f"ID: {persona[0]}, Nombre: {persona[1]}, Edad: {persona[2]}")

# Cerrar la conexión
cur.close()
conn.close()