document.getElementById('newsForm').addEventListener('submit', async (e) => {
    e.preventDefault();

    const payload = {
        title: document.getElementById('title').value,
                                                     body: document.getElementById('body').value
    };

    try {
        const response = await fetch('/news', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });

        if (response.ok) {
            alert('Notícia publicada com sucesso!');
            document.getElementById('newsForm').reset();
        } else {
            alert('Falha ao publicar a notícia no servidor.');
        }
    } catch (error) {
        alert('Erro de conexão com o API Gateway.');
    }
});
