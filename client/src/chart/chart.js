import 'chartjs-adapter-date-fns';
import Chart from 'chart.js/auto';
import { useCallback, useEffect, useRef, useState } from 'react';
import useSensorStore from '../contextStore/sensorStore';
import styles from './chart.module.scss'

const LineChart = ({ number }) => {
  const [dedupedData, setDedupedData] = useState()
  const chartRef = useRef(null);
  const chartInstance = useRef(null);

  const sensorInfo = useSensorStore((state) => state.getSensorInfo(number));
  const initialLoad = useSensorStore((state) => state.initialLoad);
  
  useEffect(() => {
    if (!initialLoad) {
      console.log('load false');
      return;
    }
    if (!sensorInfo) {
      console.log('undefined');
      return;
    }
    console.log('not undefined');
    setDedupedData([...new Set(sensorInfo.map(JSON.stringify))].map(JSON.parse));
  }, [sensorInfo, initialLoad])

  const getDayNumbers = useCallback((dates, earliestDate) => {
    const result = dates.map((date) => {
      const dayDiff = (date - earliestDate) / (1000 * 60 * 60 * 24); // difference in days
      return dayDiff;
    });

    return result;
  }, [])

  const getDedupedData = useCallback(() => {
    const data = dedupedData.map((i) => new Date(i.date_time));
    return data;
  }, [dedupedData])

  const createNewChart = useCallback((dayNumbers, dedupedData) => {
    const chart = new Chart(chartRef.current, {
      type: 'line',
      data: {
        labels: dayNumbers, // X-axis labels as day numbers
        datasets: [
          {
            label: 'Moisture Readings',
            data: dedupedData.map((i) => i.moisture), // Y-axis data as moisture readings
            borderColor: 'rgba(75, 192, 192, 1)', // Line color
            backgroundColor: 'rgba(75, 192, 192, 0.2)', // Fill color under the line
            fill: true, // Enable fill under the line
            tension: 0.4, // Line tension for smoothness
          },
        ],
      },
      options: {
        responsive: true,
        maintainAspectRatio: true,
        plugins: {
          title: {
            display: true,
            text: `Sensor Number ${number}`, // Main chart title
            font: {
              size: 18,
            },
          },
        },
        scales: {
          x: {
            type: 'linear', // Use linear scale for x-axis
            min: 0,
            max: 30,
            title: {
              display: true,
              text: 'Days',
              font: {
                size: 14,
              },
            },
          },
          y: {
            beginAtZero: true,
            min: 0,
            max: 100,
            title: {
              display: true,
              text: 'Moisture (%)',
              font: {
                size: 14,
              },
            },
          },
        },
      },
    }); 
    return chart;
  }, [number])
  
  useEffect(() => {
    if (!dedupedData || !chartRef.current) {
      return;
    }

    if (chartInstance.current) {
      return;
    }

    const dates = getDedupedData();
    const earliestDate = new Date(Math.min(...dates));
    const dayNumbers = getDayNumbers(dates, earliestDate)

    chartInstance.current = createNewChart(dayNumbers, dedupedData);

    return () => {
      if (chartInstance.current) {
        chartInstance.current.destroy();
        chartInstance.current = null;
      }
    };
  }, [dedupedData, getDedupedData, getDayNumbers, createNewChart]);

  useEffect(() => {
    if (!dedupedData || !chartInstance.current) {
      return;
    }
    
    const dates = getDedupedData();
    const earliestDate = new Date(Math.min(...dates));
    const dayNumbers = getDayNumbers(dates, earliestDate);

    chartInstance.current.data.labels = dayNumbers;
    chartInstance.current.data.datasets[0].data = dedupedData.map((i) => i.moisture);
    chartInstance.current.update();
  }, [dedupedData, getDedupedData, getDayNumbers]);

  return (
    <canvas
      ref={chartRef}
      id={`sensor${number}`}
      className={styles['single-chart']}
      style={{ backgroundColor: 'black' }} // Set canvas background to white
    ></canvas>
  );
};

export default LineChart;
