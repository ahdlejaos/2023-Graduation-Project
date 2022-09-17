USE [SkyRunner-MainServer]

DROP TABLE IF EXISTS inventory;
CREATE TABLE inventory (id_serial int PRIMARY KEY, name VARCHAR(50), quantity INTEGER);
