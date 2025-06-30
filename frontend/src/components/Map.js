import React from 'react';
import { MapContainer, TileLayer, Marker, Polyline } from 'react-leaflet';
import 'leaflet/dist/leaflet.css';
import L from 'leaflet';

import droneIconPng from '../assets/drone.png';

const droneIcon = new L.Icon({
    iconUrl: droneIconPng,
    iconSize: [40, 40],
    iconAnchor: [20, 40],
    popupAnchor: [0, -40]
});


const Map = ({ position, waypoints }) => {
  return (
    <MapContainer center={[position.latitude, position.longitude]} zoom={18} style={{ height: '500px', width: '100%' }}>
      <TileLayer
        url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
        attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
      />
      <Marker position={[position.latitude, position.longitude]} icon={droneIcon} />
      <Polyline positions={waypoints} color="blue" />
    </MapContainer>
  );
};

export default Map;