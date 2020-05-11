#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

namespace JsonUtils
{
    template <typename T>
    T fromString(const QString& s, T defaultValue)
    {
        bool ok;
        const int value = QMetaEnum::fromType<T>().keyToValue(s.toUtf8().data(), &ok);
        if (ok)
        {
            return static_cast<T>(value);
        }
        return defaultValue;
    }

    template <typename T>
    QString toString(const T& v)
    {
        return QMetaEnum::fromType<T>().valueToKey((int)v);
    }

    template <typename EnumType, size_t size>
    EnumType fromString(const QString& s, const QString (&labelArray)[size], EnumType defaultValue)
    {
        int i = 0;
        for (const QString& label : labelArray)
        {
            if (label == s)
            {
                return static_cast<EnumType>(i);
            }
            ++i;
        }
        return defaultValue;
    }

    template <typename EnumType, size_t size>
    QString toString(const EnumType& v, const QString (&labelArray)[size])
    {
        return labelArray[static_cast<int>(v)];
    }

    template <class...>
    constexpr std::false_type always_false{};
    // Converts a json value to real value with type the same as defaultValue,
    // in case json value is undefined it returns default value
    template <typename R>
    R fromJsonImpl(const QJsonValue& val, const R& defaultValue)
    {
        if (val.isUndefined())
        {
            return defaultValue;
        }
        using ValueType = R; //typename std::remove_const<typename std::remove_reference<R>::type>::type;
        if constexpr (std::is_same_v<ValueType, bool>)
        {
            return val.toBool();
        }
        else if constexpr (std::is_floating_point_v<ValueType>)
        {
            return static_cast<ValueType>(val.toDouble());
        }
        else if constexpr (std::is_integral_v<ValueType>)
        {
            return static_cast<ValueType>(val.toInt());
        }
        else if constexpr (std::is_same_v<ValueType, QString>)
        {
            return val.toString();
        }
        else if constexpr (std::is_enum_v<ValueType>)
        {
            return fromString<R>(val.toString(), defaultValue);
        }
        else
        {
            static_assert(always_false<ValueType>, "Unknown return type!");
        }
    }

    // Convert a json value of json object with name `name` to real value with type the same as defaultValue
    template <typename T>
    T fromJson(const QJsonObject& obj, const QLatin1String& name, const T& defaultValue)
    {
        const QJsonValue& val = obj[name];
        return fromJsonImpl<T>(val, defaultValue);
    }

    // Convert a json value of json array with index `index` to real value with type the same as defaultValue
    template <typename T>
    T fromJson(const QJsonArray& array, int index, const T& defaultValue)
    {
        const QJsonValue& val = array[index];
        return fromJsonImpl<T>(val, defaultValue);
    }

    // Compares `value` and `default value and if not equal set `value` to json object under `name`,
    // return `true` if object is changed
    template <typename T>
    bool toJson(QJsonObject& obj, const QLatin1String& name, const T& value, const T& defaultValue)
    {
        if (value != defaultValue)
        {
            if constexpr (std::is_enum_v<T>)
            {
                obj[name] = toString<T>(value);
            }
            else
            {
                obj[name] = value;
            }

            return true;
        }
        return false;
    }

    // Compares `value` and `default value and if not equal set `value` to json object index `index`,
    // if array is too small it resizes it up to given index,
    // if value equals to default value then forceDefaultValue used to keep array same size,
    // return `true` if object is changed
    template <typename T>
    bool toJson(QJsonArray& array, int index, const T& value, const T& defaultValue, const T& forceDefaultValue)
    {
        const bool changed = value != defaultValue;
        const T& finalValue = changed ? value : forceDefaultValue;
        while (array.size() <= index)
        {
            array.push_back(QJsonValue{});
        }
        if constexpr (std::is_enum_v<T>)
        {
            array[index] = toString<T>(finalValue);
        }
        else
        {
            array[index] = finalValue;
        }
        return changed;
    }

    template <typename T>
    T clamp(T value, T minValue, T maxValue)
    {
        if (value < minValue)
        {
            return minValue;
        }
        if (value > maxValue)
        {
            return maxValue;
        }
        return value;
    }
} // namespace JsonUtils
