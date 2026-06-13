![C++](https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white)
[![Docker](https://img.shields.io/badge/Docker-%230db7ed.svg?&logo=docker&logoColor=white)](https://hub.docker.com/r/luniphys/taskmanager)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

# Task Manager

A small project for learning **object-oriented programming (OOP)** through a simple task manager console app.

## Focus
- Classes and objects
- Encapsulation
- Inheritance
- Polymorphism

## About the App
The app is designed to manage tasks in a simple and structured way. Users can create tasks, update their details, mark them as completed, and organize them by filtering/sorting.

## Usage

Compile the file:

```bash
g++ src/taskmanager.cpp -o src/taskmanager
```

Run the file:

```bash
g++ ./src/taskmanager
```

## Docker

A Dockerfile is included to provide a reproducible runtime environment with all required dependencies.

### Build the image

From the repository root, build the Docker image with:

```bash
docker build -t taskmanager .
```

### Pull from Docker Hub

A prebuilt image is also available on [Docker Hub](https://hub.docker.com/r/luniphys/taskmanager):

```bash
docker pull luniphys/taskmanager
```

### Run the container

```bash
docker run --rm -it taskmanager
```

### Notes

- Run the container in interactive mode: ```-it```
