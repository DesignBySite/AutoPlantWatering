// sensorStore.js
import { create } from 'zustand';

const useSensorStore = create((set, get) => ({
  sensors: {}, // An object to hold data for each sensor
  initialLoad: false,
  getSensorInfo: (number) => {
    const sensors = get().sensors;
    return sensors[number];
  },
  getInitialLoad: () => {
    return get().initialLoad;
  },
  updateIndividualSensorData: (sensorData) => {
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
  initialSensorDataImport: (sensorDataArray) => {
    set((state) => {
      const newDataStore = { ...state.sensors };
  
      sensorDataArray.forEach((sensorData) => {
        const {sensorNumber, moisture, date_time } = sensorData;
        const sensor = newDataStore[sensorNumber] || [];
  
        const newDataPoint = { moisture, date_time: new Date(date_time) };
        const updatedSensorData = [...sensor, newDataPoint];
  
        updatedSensorData.sort((a, b) => a.date_time - b.date_time);
  
        newDataStore[sensorNumber] = updatedSensorData;
      });
      set(() =>  ({initialLoad: true}));
      return {
        sensors: newDataStore,
      }
    })
  }
}));

export default useSensorStore;
