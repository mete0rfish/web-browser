import { useState } from "react";

function App() {
  const [searchTerm, setSearchTerm] = useState("");
  const [content, setContent] = useState("");

  const handleSearch = async () => {
    const response = await fetch(`http://localhost:8080/search=${searchTerm}`);
    const data = await response.text();
    setContent(data);
  };

  return (
    <div>
      <input
        type="text"
        value={searchTerm}
        onChange={(e) => setSearchTerm(e.target.value)}
        placeholder="Search Google"
      />
      <button onClick={handleSearch}>Search</button>
      <div dangerouslySetInnerHTML={{ __html: content }} />
    </div>
  );
}

export default App;
