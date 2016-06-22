#pragma once

#include <mbgl/style/layer.hpp>
#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layers/raster_layer.hpp>
#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/style/conversion.hpp>
#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion/filter.hpp>
#include <mbgl/style/conversion/layer.gen.hpp>

namespace mbgl {
namespace style {
namespace conversion {

template <class V>
optional<Error> setProperty(Layer& layer, const std::string& name, const V& value, const PropertySetters<V>& setters) {
    auto it = setters.find(name);
    if (it == setters.end()) {
        return Error { "property not found" };
    }
    return it->second(layer, value);
}

template <class V>
optional<Error> setPaintProperty(Layer& layer, const std::string& name, const V& value) {
    static const PropertySetters<V> setters = makePaintPropertySetters<V>();
    return setProperty(layer, name, value, setters);
}

template <class V>
optional<Error> setLayoutProperty(Layer& layer, const std::string& name, const V& value) {
    static const PropertySetters<V> setters = makeLayoutPropertySetters<V>();
    return setProperty(layer, name, value, setters);
}

template <>
struct Converter<std::unique_ptr<Layer>> {
public:
    template <class V>
    Result<std::unique_ptr<Layer>> operator()(const V& value) const {
        if (!isObject(value)) {
            return Error { "layer must be an object" };
        }

        auto idValue = objectMember(value, "id");
        if (!idValue) {
            return Error { "layer must have an id" };
        }

        optional<std::string> id = toString(*idValue);
        if (!id) {
            return Error { "layer id must be a string" };
        }

        auto typeValue = objectMember(value, "type");
        if (!typeValue) {
            return Error { "layer must have a type" };
        }

        optional<std::string> type = toString(*typeValue);
        if (!type) {
            return Error { "layer type must be a string" };
        }

        Result<std::unique_ptr<Layer>> converted;

        if (*type == "fill") {
            converted = convertVectorLayer<FillLayer>(*id, value);
        } else if (*type == "line") {
            converted = convertVectorLayer<LineLayer>(*id, value);
        } else if (*type == "circle") {
            converted = convertVectorLayer<CircleLayer>(*id, value);
        } else if (*type == "symbol") {
            converted = convertVectorLayer<SymbolLayer>(*id, value);
        } else if (*type == "raster") {
            converted = convertRasterLayer(*id, value);
        } else if (*type == "background") {
            converted = convertBackgroundLayer(*id, value);
        } else {
            return Error { "invalid layer type" };
        }

        if (!converted) {
            return converted;
        }

        std::unique_ptr<Layer> layer = std::move(*converted);

        auto minzoomValue = objectMember(value, "minzoom");
        if (minzoomValue) {
            optional<float> minzoom = toNumber(*minzoomValue);
            if (!minzoom) {
                return Error { "minzoom must be numeric" };
            }
            layer->setMinZoom(*minzoom);
        }

        auto maxzoomValue = objectMember(value, "maxzoom");
        if (maxzoomValue) {
            optional<float> maxzoom = toNumber(*maxzoomValue);
            if (!maxzoom) {
                return Error { "maxzoom must be numeric" };
            }
            layer->setMaxZoom(*maxzoom);
        }

        auto layoutValue = objectMember(value, "layout");
        if (layoutValue) {
            if (!isObject(*layoutValue)) {
                return Error { "layout must be an object" };
            }
            eachMember(*layoutValue, [&] (const std::string& k, const V& v) {
                return setLayoutProperty(*layer, k, v);
            });
        }

        return std::move(layer);
    }

private:
    template <class LayerType, class V>
    Result<std::unique_ptr<Layer>> convertVectorLayer(const std::string& id, const V& value) const {
        auto sourceValue = objectMember(value, "source");
        if (!sourceValue) {
            return Error { "layer must have a source" };
        }

        optional<std::string> source = toString(*sourceValue);
        if (!source) {
            return Error { "layer source must be a string" };
        }

        std::unique_ptr<LayerType> layer = std::make_unique<LayerType>(id, *source);

        auto sourceLayerValue = objectMember(value, "source-layer");
        if (sourceLayerValue) {
            optional<std::string> sourceLayer = toString(*sourceLayerValue);
            if (!sourceLayer) {
                return Error { "layer source-layer must be a string" };
            }
            layer->setSourceLayer(*sourceLayer);
        }

        auto filterValue = objectMember(value, "filter");
        if (filterValue) {
            Result<Filter> filter = convert<Filter>(*filterValue);
            if (!filter) {
                return filter.error();
            }
            layer->setFilter(*filter);
        }

        return std::move(layer);
    }

    template <class V>
    Result<std::unique_ptr<Layer>> convertRasterLayer(const std::string& id, const V& value) const {
        auto sourceValue = objectMember(value, "source");
        if (!sourceValue) {
            return Error { "layer must have a source" };
        }

        optional<std::string> source = toString(*sourceValue);
        if (!source) {
            return Error { "layer source must be a string" };
        }

        return std::make_unique<RasterLayer>(id, *source);
    }

    template <class V>
    Result<std::unique_ptr<Layer>> convertBackgroundLayer(const std::string& id, const V&) const {
        return std::make_unique<BackgroundLayer>(id);
    }
};

} // namespace conversion
} // namespace style
} // namespace mbgl
