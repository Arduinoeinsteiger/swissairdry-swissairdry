FROM python:3.11-slim

WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc \
    python3-dev \
    libpq-dev \
    curl \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.api.txt .
RUN pip install --no-cache-dir -r requirements.api.txt

COPY . .
RUN mkdir -p logs

ENV PORT=5000

CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "5000"] 