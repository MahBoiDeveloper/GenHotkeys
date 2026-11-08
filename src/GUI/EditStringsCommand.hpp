#pragma once
#include <QUndoCommand>
#include <QVector>
#include <utility>

#include "../Parsers/CSFParser.hpp"

/// @brief Restores complete string values through the existing parser API.
class EditStringsCommand final : public QUndoCommand
{
public:
    struct Change
    {
        QString label;
        QString before;
        QString after;
    };

    EditStringsCommand(const QString& description, QVector<Change> changes)
        : QUndoCommand(description), changes{std::move(changes)} {}

    void undo() override
    {
        for (const auto& change : changes)
            CSF_PARSER->SetStringValue({change.label, change.before});
    }

    void redo() override
    {
        for (const auto& change : changes)
            CSF_PARSER->SetStringValue({change.label, change.after});
    }

private:
    const QVector<Change> changes;
};
