CREATE DATABASE measurements;
USE measurements;
CREATE TABLE rawdata (
 id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
 timestamp TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP(),
 groupid INT NOT NULL DEFAULT 0,
 from_mac VARCHAR(32) NOT NULL DEFAULT '0',
 to_mac VARCHAR(32) NOT NULL DEFAULT '0',
 sensorvalue_a FLOAT NOT NULL DEFAULT 0,
 sensorvalue_b FLOAT NOT NULL DEFAULT 0,
 sensorvalue_c FLOAT NOT NULL DEFAULT 0,
 sensorvalue_d FLOAT NOT NULL DEFAULT 0,
 sensorvalue_e FLOAT NOT NULL DEFAULT 0,
 sensorvalue_f VARCHAR(256) NOT NULL DEFAULT '0'
);
CREATE USER 'dbaccess_rw'@'%' IDENTIFIED WITH mysql_native_password BY 'salasana123';
GRANT ALL ON measurements.* TO 'dbaccess_rw'@'%';
CREATE USER 'dbaccess_ro'@'%' IDENTIFIED WITH mysql_native_password BY 'salasana456';
GRANT SELECT ON measurements.* TO 'dbaccess_ro'@'%';
FLUSH PRIVILEGES;