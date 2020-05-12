#pragma once

class ParameterModel;

// interface class for bound widgets, widgets displaying a ParameterModel

class BoundWidget
{
public:
    virtual const ParameterModel* getModel() const = 0;
    virtual void updateFromModel() = 0;
};

Q_DECLARE_INTERFACE(BoundWidget, "BoundWidget");
