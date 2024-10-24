// sensorStore.js
import { create } from 'zustand';

const useSensorStore = create((set, get) => ({
  sensors: {}, // An object to hold data for each sensor
  updateSensorData: (sensorData) => {
    const { sensorNumber, moisture, date_time } = sensorData;

    set((state) => {
      const sensor = state.sensors[sensorNumber] || [];

      const updatedSensorData = [...sensor, { moisture, date_time: new Date(date_time) }]

      return {
        sensors: {
          ...state.sensors,
          [sensorNumber]: updatedSensorData,
        },
      };
    });
  },
}));

export default useSensorStore;
