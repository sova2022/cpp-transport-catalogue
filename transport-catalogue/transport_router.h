#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace transport_router {

	constexpr static double METERS_PER_KM = 1000.0;
	constexpr static double MIN_PER_HOUR = 60.0;

	struct RouteItem {
		std::string edge_name;
		int span_count = 0;
		double time = 0.0;
		graph::EdgeType type;
	};

	struct RouteData {
		double total_time = 0.0;
		std::vector<RouteItem> items;
		bool founded = false;
	};

	struct VertexWithMirror {
		size_t wait;
		size_t travel;
	};

	class TransportRouter {

		using Router = graph::Router<double>;
		using Graph = graph::DirectedWeightedGraph<double>;
		using Vertexes = std::unordered_map<std::string_view, VertexWithMirror>;

	public:
		TransportRouter(const TransportCatalogue& db);

		void SetRoutingSettings(const RoutingSettings settings);
		RoutingSettings GetRoutingSettings();
		const RouteData CalculateRoute(const std::string_view from, const std::string_view to);

	private:
		RoutingSettings settings_;
		Graph graph_;
		std::unique_ptr<Router> router_ = nullptr;
		const TransportCatalogue& tc_;
		Vertexes vertexes_;

		void BuildGraph();
	};

} // namespace transport_router