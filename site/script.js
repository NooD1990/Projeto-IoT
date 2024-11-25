async function buscarUltimoDocumento() {
    try {
      const response = await fetch('http://localhost:3000/documentos/ultimo');
      const data = await response.json();
  
      const ultimoDocumento = data.documento;
  
      if (!ultimoDocumento) {
        console.error('Nenhum doc encontrado');
        return;
      }
  
      console.log('dads recebidos:', ultimoDocumento);
  
      //  atualiza o front end com os elementos
      document.getElementById('data-area').textContent = `Data: ${new Date(ultimoDocumento._id).toLocaleString()}`;
      document.getElementById('temp-value').textContent = `${ultimoDocumento.temperatura} °C`;
      document.getElementById('humidity-value').textContent = `${ultimoDocumento.umidade} %`;
      document.getElementById('gas-level-value').textContent = `${ultimoDocumento.nivelGas} ppm`;
      document.getElementById('gps-value').textContent = ultimoDocumento.gps;
      document.getElementById('light-level-value').textContent = `${ultimoDocumento.nivelLuz} lux`;
      document.getElementById('panic-button-value').textContent = ultimoDocumento.botaoPanico ? 'Ativado' : 'Desativado';
    } catch (error) {
      console.error('erro ao buscar dados da API:', error);
    }
  }
  
  document.addEventListener('DOMContentLoaded', () => {
    buscarUltimoDocumento();
    setInterval(buscarUltimoDocumento, 1000); // atualiza a cada 1 sec
  });
  