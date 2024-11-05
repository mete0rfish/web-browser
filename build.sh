sudo apt-get update
sudo apt-get install gcc git curl libcurl4-openssl-dev
# curl -fsSL https://fnm.vercel.app/install | bash
# source ~/.bashrc
# fnm use --install-if-missing 21
gcc ./socket/server.c -o ./socket/server -lcurl
npm i
npm run dev
