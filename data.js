let data = [];
        let currentPage = 1;
        const rowsPerPage = 20;

        // Function untuk mendapatkan data dari Google Spreadsheet dan memperbarui UI
        async function fetchData() {
            const response = await fetch('https://script.google.com/macros/s/AKfycbw23vE7HATyJBHlxQOh5Wo_jux-tBhBeDbIgi7e6Y7F-tV69CyV48RbasmV0dFEZk4g/exec'); // Ganti dengan URL Google Apps Script
            const result = await response.json();
            data = result.history;

            document.getElementById('flowRate').textContent = result.flowRate + ' L/min';
            document.getElementById('totalLiters').textContent = result.totalLiters + ' L';
            document.getElementById('totalCost').textContent = 'Rp ' + result.totalCost;

            renderTable();
        }

        function renderTable() {
            const historyTable = document.getElementById('historyTable');
            historyTable.innerHTML = '';

            // Pagination
            const start = (currentPage - 1) * rowsPerPage;
            const end = start + rowsPerPage;
            const paginatedData = data.slice(start, end);

            // Menampilkan data pada halaman tertentu
            paginatedData.forEach(entry => {
                const row = `<tr>
                    <td>${entry.date}</td>
                    <td>${entry.flowRate}</td>
                    <td>${entry.totalLiters}</td>
                    <td>${entry.totalCost}</td>
                </tr>`;
                historyTable.innerHTML += row;
            });

            // Update tombol prev/next
            document.getElementById('prevPage').disabled = currentPage === 1;
            document.getElementById('nextPage').disabled = end >= data.length;

            // Hitung total di bawah tabel
            calculateSum(paginatedData);
        }

        function calculateSum(paginatedData) {
            let sumFlowRate = 0;
            let sumTotalLiters = 0;
            let sumTotalCost = 0;

            paginatedData.forEach(entry => {
                sumFlowRate += parseFloat(entry.flowRate);
                sumTotalLiters += parseFloat(entry.totalLiters);
                sumTotalCost += parseFloat(entry.totalCost);
            });

            document.getElementById('sumFlowRate').textContent = sumFlowRate.toFixed(2) + ' L/min';
            document.getElementById('sumTotalLiters').textContent = sumTotalLiters.toFixed(2) + ' L';
            document.getElementById('sumTotalCost').textContent = 'Rp ' + sumTotalCost.toFixed(2);
        }

        // Event listener untuk tombol prev/next
        document.getElementById('prevPage').addEventListener('click', () => {
            if (currentPage > 1) {
                currentPage--;
                renderTable();
            }
        });

        document.getElementById('nextPage').addEventListener('click', () => {
            if (currentPage * rowsPerPage < data.length) {
                currentPage++;
                renderTable();
            }
        });

        // Panggil fetchData untuk mendapatkan dan menampilkan data secara real-time
        fetchData();
        setInterval(fetchData, 10000);  // Update setiap 10 detik