# WebCamRecognizer

## Описание проекта
GUI утилита для подсчета количества стульев в видеопотоке, получаемом с веб-камеры.

## Технологии, библиотеки и зависимости
Проект написан на C++ с использованием Qt. Для выполнения задачи распознавания используется библиотека OpenCV с предобученной нейронной сетью заимствованной [отсюда](https://proglib.io/p/real-time-object-detection/).

## Сборка
Сборка осуществляется с помощью qmake, проще всего это делать из QtCreator. Зависимости:
* Qt не ниже 5.7.1;
* OpenCV не ниже 4.1, собранная c дополнительными модулями [opencv_contrib](https://github.com/Itseez/opencv_contrib).

К сожалению, скорее всего потребуется править .pro файл проекта, чтобы указать пути для библиотеки OpenCV.
Если при сборке не удается обнаружить файл "opencv2/tracking.hpp", то, скорее всего, Ваша библиотека OpenCV была собрана без opencv_contrib, подробнее [здесь](https://devtalk.nvidia.com/default/topic/1043027/jetson-agx-xavier/opencv-tracking-hpp-missing-solved-/).
Для создания распространяемой сборки для Windows необходимо выполнить следующие шаги:
* Собрать проект с помощью qmake. В результате создается файл WebCamRecognizer.exe - искомая GUI утилита;
* Положить WebCamRecognizer.exe в отдельную папку с названием сборки, и воспользоваться
  [windeployqt](http://doc.qt.io/qt-5/windows-deployment.html) для копирования необходимых библиотек;
* Скомпилировать файл переводов "WebCamRecognizer_ru.ts" с помощью [lrelease](http://doc.qt.io/qt-5/linguist-manager.html). Полученный .qm файл скопировать в папку "translations" сборки.

## Использование

Утилита делает, главным образом, две вещи:
* Получает видеопоток с выбранной камеры (режим видеоискателя) и отображает его на интерфейсе;
* По запросу пользователя делает одиночное или непрерывное распознавание стульев в видеопотоке и подсчетом их количества.

Выбор камеры осуществляется в главном меню ("Devices"). После выбора камеры автоматически начинается считывание видеопотока и его отображение.
На панели быстрого доступа или в меню можно включить одиночное или непрерывное распознавание. В этом случае из видеопотока будут выбираться кадры, на которых будут отмечены стулья (если они распознаются) и их количество.
Настройки всего две:
* "Exposure Compensation" (экспозиция) - относится к настройкам веб-камеры. Имеет эффект не на всех устройствах.
* "Confidence Threshold" (порог уверенности) - относится к алгоритму распознавания. Отсекает объекты, уверенность распознания которых оказался ниже указанного.

Также можно управлять фокусировкой камеры (работает не на всех устройствах).

## Глобальные TODO

* Перейти на сборку с помощью CMake;
* Создать собственную нейронную сеть для распознавания образов;
* Протестировать на различных камерах "Exposure Compensation" (экспозиция) и фокусировку. Не во всех камерах поддерживается.

## Автор

* **Тимофей Абрамов** - *[timohamail@inbox.ru](mailto://timohamail@inbox.ru)*.