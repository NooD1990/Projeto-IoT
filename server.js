const express = require("express");
const { MongoClient } = require("mongodb");
const cors = require("cors");
const bodyParser = require("body-parser");

// Configurações do MongoDB
const minhaSenha = "123";
const uri = `mongodb+srv://joaoteste:${minhaSenha}@iotcluster.a70ey.mongodb.net/?retryWrites=true&w=majority&appName=IotCluster`;
const client = new MongoClient(uri);

const app = express();

// Middlewares
app.use(cors()); // Permite todas as origens
app.use(bodyParser.json()); // Lida com JSON no corpo das requisições

// Conecta ao banco de dados
let collection;
client.connect().then(() => {
  const db = client.db("iot");
  collection = db.collection("gps");
  console.log("Conectado ao MongoDB");
});

// Rotas

// GET /documentos: Obtém todos os documentos
app.get("/documentos", async (req, res) => {
  try {
    const documentos = await collection.find({}).toArray();
    documentos.forEach((doc) => (doc._id = doc._id.toString())); // Converte o _id para string
    res.json({ documentos });
  } catch (error) {
    res.status(500).json({ error: "Erro ao buscar documentos" });
  }
});

// POST /documentos: Insere um novo documento
app.post("/documentos", async (req, res) => {
  try {
    const novoDocumento = req.body;
    const resultado = await collection.insertOne(novoDocumento);
    res.json({ message: "Documento inserido", id: resultado.insertedId.toString() });
  } catch (error) {
    res.status(500).json({ error: "Erro ao inserir documento" });
  }
});

// GET /documentos/ultimo: opega o ultimo elemento do DB
app.get("/documentos/ultimo", async (req, res) => {
  try {
    const ultimoDocumento = await collection.findOne({}, { sort: { _id: -1 } }); // Ordena por _id decrescente
    if (ultimoDocumento) {
      ultimoDocumento._id = ultimoDocumento._id.toString();
    }
    res.json({ documento: ultimoDocumento });
  } catch (error) {
    res.status(500).json({ error: "Erro ao buscar o último documento" });
  }
});

// Inicia o servidor
app.listen(PORT, () => {
  console.log(`Servidor rodando em http://localhost:3000`);
});
