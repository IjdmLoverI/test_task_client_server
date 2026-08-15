FROM python:3.12-slim

# Unbuffered output: otherwise the menu prompts sit in the buffer and an
# interactive user is left looking at a blank screen.
ENV PYTHONUNBUFFERED=1 \
    PYTHONDONTWRITEBYTECODE=1

WORKDIR /app

# Dependencies in their own layer: editing the code does not reinstall pip packages.
COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY src ./src

RUN useradd --system --create-home --uid 10001 appuser
USER appuser

ENTRYPOINT ["python", "-m", "src"]
