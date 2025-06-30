import axios from 'axios';

const API_URL = 'http://localhost:8080';

export const sendCommand = (command, data = null) => {
  if (command === 'start' && data) {
    return axios.post(`${API_URL}/start`, data, {
      headers: { 'Content-Type': 'text/plain' },
    });
  }
  return axios.get(`${API_URL}/${command}`);
};

export const getTelemetry = () => {
  return axios.get(`${API_URL}/telemetry`);
};

export const getMissionProgress = () => {
  return axios.get(`${API_URL}/mission_progress`);
};

export const getBattery = () => {
  return axios.get(`${API_URL}/battery`);
};

export const getAltitude = () => {
    return axios.get(`${API_URL}/altitude`);
};

export const getHeading = () => {
    return axios.get(`${API_URL}/heading`);
};