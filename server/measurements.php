<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);

$servername = "DB_HOST";
$username = "DB_USER";
$password = "DB_PASS";
$dbname = "DB_NAME";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Query
$sql = "SELECT id, timestamp, sensor_direction, sensorvalue_x, sensorvalue_y, sensorvalue_z FROM rawdata ORDER BY id DESC";
$result = $conn->query($sql);

// Output
if ($result === false) {
    die("SQL Error: " . $conn->error);
}

if ($result->num_rows > 0) {
    echo "<table border='1' style='border-collapse: collapse;'>";
    echo "<tr><th>ID</th><th>Timestamp</th><th>Direction</th><th>X</th><th>Y</th><th>Z</th></tr>";
    while($row = $result->fetch_assoc()) {
        echo "<tr>
                <td>{$row['id']}</td>
                <td>{$row['timestamp']}</td>
                <td>{$row['sensor_direction']}</td>
                <td>{$row['sensorvalue_x']}</td>
                <td>{$row['sensorvalue_y']}</td>
                <td>{$row['sensorvalue_z']}</td>
              </tr>";
    }
    echo "</table>";
} else {
    echo "No results found.";
}

$conn->close();
?>
