#include "Application.h"
#include <QEvent>
#include <QFileOpenEvent>

/**
 * @brief Конструктор
 * @param[inout] argc Количество аргументов
 * @param[inout] argv Массив C-строк - значений аргументов
 */
Application::Application(int & argc, char ** argv)
    : QApplication(argc, argv)
{}

/**
 * @brief Получить имя последнего файла, который пришел в QFileOpenEvent
 * @return Имя последнего файла, который пришел в QFileOpenEvent
 */
const QString & Application::getLastOpenFilename() const
{
    return m_lastOpenFilename;
}

/**
 * @brief Узнать, сохранено ли имя последнего файла, который пришел в QFileOpenEvent
 * @return true - имя сохранено, false - имя не сохранено
 */
bool Application::hasLastOpenFilename() const
{
    return !m_lastOpenFilename.isEmpty();
}

/**
 * @brief Обработчик событий
 * @param[in] event Событие
 * @return true - событие распознано и обработано, false - иначе
 */
bool Application::event(QEvent * event)
{
    if(event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent * fileOpenEvent = static_cast<QFileOpenEvent *>(event);
        if(fileOpenEvent)
        {
            m_lastOpenFilename = fileOpenEvent->file();
            if(hasLastOpenFilename())
            {
                emit openFileEvent(m_lastOpenFilename);
                return true;
            }
        }
    }
    return QApplication::event(event);
}
