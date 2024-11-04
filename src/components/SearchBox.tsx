import { useContext, useState } from "react";
import { HtmlContext } from "../store/HtmlStorage";

export default function SearchBox() {
  const [searchTerm, setSearchTerm] = useState("");
  const [viewTerm, setViewTerm] = useState("");
  const { content, setContent } = useContext(HtmlContext); // Context에서 setContent 가져오기

  const handleSearch = async () => {
    try {
      const response = await fetch(
        `http://localhost:8080/search=${searchTerm}`
      );
      const data = await response.text();

      setContent(data);
    } catch (error) {
      console.error("Error fetching HTML:", error);
      setContent("Failed to fetch content.");
    }
  };

  const handleURLSearch = async () => {
    try {
      const response = await fetch(`http://localhost:8080/view=${viewTerm}`);
      const data = await response.text();

      setContent(data);
    } catch (error) {
      console.error("Error fetching HTML:", error);
      setContent("Failed to fetch content.");
    }
  };

  const handleRefresh = () => {
    setSearchTerm("");
    setViewTerm("");
    setContent("");
  };

  const openModal = () => {
    const modal = document.getElementById(
      "my_modal_3"
    ) as HTMLDialogElement | null;
    if (modal) {
      modal.showModal();
    }
  };

  return (
    <>
      {!content ? (
        <div
          className={`flex flex-col p-6 rounded-lg shadow-lg bg-white items-center top-4`}
        >
          <h1 className="text-3xl font-bold text-gray-900 mb-8 text-center tracking-wide">
            WebLinker
          </h1>

          {/* 검색용 섹션 */}
          <div className="flex w-full max-w-md gap-3 mb-6">
            <input
              type="text"
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
              placeholder="Enter search term"
              className="flex-1 p-3 border border-gray-200 rounded-xl shadow-sm focus:ring-2 focus:ring-blue-500 outline-none bg-gray-50 text-gray-700"
            />
            <button
              onClick={handleSearch}
              className="px-4 py-2 bg-blue-600 text-white rounded-xl shadow-md hover:bg-blue-700 transition duration-300"
            >
              Search
            </button>
          </div>

          {/* URL 검색용 섹션 */}
          <div className="flex w-full max-w-md gap-3 mb-6">
            <input
              type="text"
              value={viewTerm}
              onChange={(e) => setViewTerm(e.target.value)}
              placeholder="Enter URL term"
              className="flex-1 p-3 border border-gray-200 rounded-xl shadow-sm focus:ring-2 focus:ring-green-500 outline-none bg-gray-50 text-gray-700"
            />
            <button
              onClick={handleURLSearch}
              className="px-4 py-2 bg-green-600 text-white rounded-xl shadow-md hover:bg-green-700 transition duration-300"
            >
              URL Search
            </button>
          </div>

          <button
            onClick={handleRefresh}
            className="w-full px-4 py-2 bg-red-600 text-white rounded-xl shadow-md hover:bg-red-700 transition duration-300"
          >
            Reset
          </button>
        </div>
      ) : (
        <>
          <button
            className="fixed top-10 right-10 hover:scale-125 transition-transform z-50"
            onClick={openModal}
          >
            <svg
              xmlns="http://www.w3.org/2000/svg"
              x="0px"
              y="0px"
              width="50"
              height="50"
              viewBox="0 0 50 50"
            >
              <path d="M 21 3 C 11.621094 3 4 10.621094 4 20 C 4 29.378906 11.621094 37 21 37 C 24.710938 37 28.140625 35.804688 30.9375 33.78125 L 44.09375 46.90625 L 46.90625 44.09375 L 33.90625 31.0625 C 36.460938 28.085938 38 24.222656 38 20 C 38 10.621094 30.378906 3 21 3 Z M 21 5 C 29.296875 5 36 11.703125 36 20 C 36 28.296875 29.296875 35 21 35 C 12.703125 35 6 28.296875 6 20 C 6 11.703125 12.703125 5 21 5 Z"></path>
            </svg>
          </button>

          {/* 모달 */}
          <dialog id="my_modal_3" className="modal">
            <form method="dialog">
              <div
                className={`flex flex-col p-6 rounded-lg shadow-lg bg-white items-center top-4`}
              >
                <h1 className="text-3xl font-bold text-gray-900 mb-8 text-center tracking-wide">
                  WebLinker
                </h1>

                {/* 검색용 섹션 */}
                <div className="flex w-full max-w-md gap-3 mb-6">
                  <input
                    type="text"
                    value={searchTerm}
                    onChange={(e) => setSearchTerm(e.target.value)}
                    placeholder="Enter search term"
                    className="flex-1 p-3 border border-gray-200 rounded-xl shadow-sm focus:ring-2 focus:ring-blue-500 outline-none bg-gray-50 text-gray-700"
                  />
                  <button
                    onClick={handleSearch}
                    className="px-4 py-2 bg-blue-600 text-white rounded-xl shadow-md hover:bg-blue-700 transition duration-300"
                  >
                    Search
                  </button>
                </div>

                {/* URL 검색용 섹션 */}
                <div className="flex w-full max-w-md gap-3 mb-6">
                  <input
                    type="text"
                    value={viewTerm}
                    onChange={(e) => setViewTerm(e.target.value)}
                    placeholder="Enter URL term"
                    className="flex-1 p-3 border border-gray-200 rounded-xl shadow-sm focus:ring-2 focus:ring-green-500 outline-none bg-gray-50 text-gray-700"
                  />
                  <button
                    onClick={handleURLSearch}
                    className="px-4 py-2 bg-green-600 text-white rounded-xl shadow-md hover:bg-green-700 transition duration-300"
                  >
                    URL Search
                  </button>
                </div>

                <button
                  onClick={handleRefresh}
                  className="w-full px-4 py-2 bg-red-600 text-white rounded-xl shadow-md hover:bg-red-700 transition duration-300"
                >
                  Reset
                </button>
              </div>
            </form>
          </dialog>
        </>
      )}
    </>
  );
}
