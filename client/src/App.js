import React, { useEffect, useState } from 'react';

function App() {
  const [messages, setMessages] = useState([]);

  useEffect(() => {
    const eventSource = new EventSource('http://localhost:3050/events');
    eventSource.onmessage = (event) => {
      const newMessage = JSON.parse(event.data);
      setMessages((prevMessages) => [...prevMessages, newMessage]);
    };

    return () => {
      eventSource.close();
    };
  }, []);

  return (
    <div>
      <p>Messages</p>
      {messages.map((msg, index) => (
        <div key={index}>{msg.message}</div>
      ))}
    </div>
  );
}

export default App;