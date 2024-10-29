import { useEffect } from 'react';
import useSensorStore from '../contextStore/sensorStore';

const useSSEListener = () => {
  const updateSensorData = useSensorStore((state) => state.updateIndividualSensorData);

  useEffect(() => {
    const eventSource = new EventSource('http://localhost:3050/events');

    eventSource.addEventListener('sensorUpdate', (event) => {
      const newData = JSON.parse(event.data);
      updateSensorData(newData);
      console.log('updated sensor');
    });

    eventSource.onerror = (err) => {
      console.error('EventSource failed:', err);
      eventSource.close();
    };

    return () => {
      eventSource.close();
    };
  }, [updateSensorData]);
};

export default useSSEListener;
