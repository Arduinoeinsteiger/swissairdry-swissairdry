from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

app = FastAPI(
    title="SwissAirDry API",
    description="API für das SwissAirDry System",
    version="0.1.0"
)

# CORS-Einstellungen
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/")
async def root():
    return {"message": "SwissAirDry API läuft"}

@app.get("/health")
async def health_check():
    return {"status": "healthy"} 