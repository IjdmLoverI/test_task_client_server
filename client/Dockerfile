FROM python:3.12-slim

# Вывод не буферизуется: иначе приглашения меню застревают в буфере
# и в интерактивном режиме пользователь видит пустой экран.
ENV PYTHONUNBUFFERED=1 \
    PYTHONDONTWRITEBYTECODE=1

WORKDIR /app

# Зависимости отдельным слоем: правка кода не заставляет переустанавливать pip-пакеты.
COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY src ./src

RUN useradd --system --create-home --uid 10001 appuser
USER appuser

ENTRYPOINT ["python", "-m", "src"]
