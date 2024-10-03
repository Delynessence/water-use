// Function untuk mendapatkan data dari Google Spreadsheet dan memperbarui UI
async function fetchData() {
    const response = await fetch('https://script.google.com/macros/s/AKfycbxI8UXRKseJaF7mgDpQqJfFtHqiERT_L8kryvlW6uTjK7eAZ_wpV6OC80fHx5e4dLrzBw/exec');
    const data = await response.json();
    document.getElementById('flowRate').textContent = data.flowRate + ' L/min';
    document.getElementById('totalLiters').textContent = data.totalLiters + ' L';
    document.getElementById('totalCost').textContent = 'Rp ' + data.totalCost;

    const historyTable = document.getElementById('historyTable');
    historyTable.innerHTML = '';
    data.history.forEach(entry => {
        const row = `<tr>
            <td>${entry.date}</td>
            <td>${entry.flowRate}</td>
            <td>${entry.totalLiters}</td>
            <td>${entry.totalCost}</td>
        </tr>`;
        historyTable.innerHTML += row;
    });
}

setInterval(fetchData, 10000);  // Update setiap 10 detik
