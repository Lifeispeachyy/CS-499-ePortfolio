"""
CRUD_Python_Module.py
Provides create and read operations for the AAC MongoDB database.
"""

from pymongo import MongoClient
from pymongo.errors import PyMongoError


class AnimalShelterCRUD:
    """
    CRUD class for interacting with the AAC MongoDB animals collection.
    Supports 'create' and 'read' operations.
    """

    def __init__(self, username, password, host="localhost", port=27017, db="aac", collection="animals"):
        """
        Initializes a MongoDB client and connects to the specified database and collection.
        """
        try:
            connection_string = f"mongodb://{username}:{password}@{host}:{port}"
            self.client = MongoClient(connection_string)

            self.database = self.client[db]
            self.collection = self.database[collection]

        except PyMongoError as e:
            print(f"Error connecting to MongoDB: {e}")
            self.client = None

    # -------------------------------------------------------------
    # ADVANCED READ WITH PROJECTION
    # -------------------------------------------------------------
    def read_with_projection(self, query, projection):
        """
        Queries documents and returns only selected fields.
        This improves database efficiency by limiting unnecessary data retrieval.
        """
        if query is None or type(query) is not dict:
            print("Invalid query format. Must be a dictionary.")
            return []

        if projection is None or type(projection) is not dict:
            print("Invalid projection format. Must be a dictionary.")
            return []

        try:
            results = list(self.collection.find(query, projection))
            return results
        except PyMongoError as e:
            print(f"Error querying documents with projection: {e}")
            return []

    # -------------------------------------------------------------
    # COUNT DOCUMENTS
    # -------------------------------------------------------------
    def count_documents(self, query):
        """
        Counts documents that match a MongoDB query.
        This supports dashboard summary statistics.
        """
        if query is None or type(query) is not dict:
            print("Invalid query format. Must be a dictionary.")
            return 0

        try:
            return self.collection.count_documents(query)
        except PyMongoError as e:
            print(f"Error counting documents: {e}")
            return 0

    # -------------------------------------------------------------
    # AGGREGATION PIPELINE
    # -------------------------------------------------------------
    def aggregate(self, pipeline):
        """
        Runs a MongoDB aggregation pipeline.
        This supports advanced database analysis such as grouping,
        counting, and calculating summary information.
        """
        if pipeline is None or type(pipeline) is not list:
            print("Invalid pipeline format. Must be a list.")
            return []

        try:
            results = list(self.collection.aggregate(pipeline))
            return results
        except PyMongoError as e:
            print(f"Error running aggregation pipeline: {e}")
            return []

    # -------------------------------------------------------------
    # CREATE INDEXES
    # -------------------------------------------------------------
    def create_indexes(self):
        """
        Creates indexes on commonly queried fields to improve database performance.
        """
        try:
            self.collection.create_index("breed")
            self.collection.create_index("animal_type")
            self.collection.create_index("sex_upon_outcome")
            self.collection.create_index("age_upon_outcome_in_weeks")
            return True
        except PyMongoError as e:
            print(f"Error creating indexes: {e}")
            return False

    # -------------------------------------------------------------
    # C - CREATE
    # -------------------------------------------------------------
    def create(self, data):
        """
        Inserts a document into the MongoDB collection.
        Args:
            data (dict): Key/value pairs representing a MongoDB document.
        Returns:
            True if successful, False otherwise.
        """
        if data is None or type(data) is not dict:
            print("Invalid data format. Must be a dictionary.")
            return False

        try:
            result = self.collection.insert_one(data)
            return result.acknowledged
        except PyMongoError as e:
            print(f"Error inserting document: {e}")
            return False

    # -------------------------------------------------------------
    # R - READ
    # -------------------------------------------------------------
    def read(self, query):
        """
        Queries documents in the MongoDB collection.
        Args:
            query (dict): Key/value lookup pairs for find().
        Returns:
            List of matching documents, or an empty list if failed.
        """
        if query is None or type(query) is not dict:
            print("Invalid query format. Must be a dictionary.")
            return []

        try:
            results = list(self.collection.find(query))
            return results
        except PyMongoError as e:
            print(f"Error querying documents: {e}")
            return []

    # -------------------------------------------------------------
    # RESCUE TYPE QUERY HELPERS
    # -------------------------------------------------------------
    def get_rescue_candidates(self, rescue_type):
        """
        Returns animals that match predefined Grazioso Salvare rescue criteria.
        This improves modularity by centralizing rescue-specific MongoDB queries.
        """
        if rescue_type is None or type(rescue_type) is not str:
            print("Invalid rescue type. Must be a string.")
            return []

        rescue_type = rescue_type.lower().strip()

        rescue_queries = {
            "water": {
                "animal_type": "Dog",
                "breed": {
                    "$in": [
                        "Labrador Retriever Mix",
                        "Chesapeake Bay Retriever",
                        "Newfoundland"
                    ]
                },
                "sex_upon_outcome": "Intact Female",
                "age_upon_outcome_in_weeks": {"$gte": 26, "$lte": 156}
            },
            "mountain": {
                "animal_type": "Dog",
                "breed": {
                    "$in": [
                        "German Shepherd",
                        "Alaskan Malamute",
                        "Old English Sheepdog",
                        "Siberian Husky",
                        "Rottweiler"
                    ]
                },
                "sex_upon_outcome": "Intact Male",
                "age_upon_outcome_in_weeks": {"$gte": 26, "$lte": 156}
            },
            "disaster": {
                "animal_type": "Dog",
                "breed": {
                    "$in": [
                        "Doberman Pinscher",
                        "German Shepherd",
                        "Golden Retriever",
                        "Bloodhound",
                        "Rottweiler"
                    ]
                },
                "sex_upon_outcome": "Intact Male",
                "age_upon_outcome_in_weeks": {"$gte": 20, "$lte": 300}
            }
        }

        if rescue_type not in rescue_queries:
            print("Invalid rescue type. Choose water, mountain, or disaster.")
            return []

        return self.read(rescue_queries[rescue_type])

    # -------------------------------------------------------------
    # BREED SUMMARY AGGREGATION
    # -------------------------------------------------------------
    def get_breed_summary(self, query=None):
        """
        Uses MongoDB aggregation to summarize matching dogs by breed.
        This supports dashboard analytics and more advanced reporting.
        """
        if query is None:
            query = {}

        if type(query) is not dict:
            print("Invalid query format. Must be a dictionary.")
            return []

        pipeline = [
            {"$match": query},
            {"$group": {"_id": "$breed", "count": {"$sum": 1}}},
            {"$sort": {"count": -1}}
        ]

        return self.aggregate(pipeline)

    # -------------------------------------------------------------
    # RESCUE SUMMARY COUNTS
    # -------------------------------------------------------------
    def get_rescue_summary_counts(self):
        """
        Returns count totals for each rescue category.
        This provides a dashboard-ready summary of rescue candidate groups.
        """
        water_count = len(self.get_rescue_candidates("water"))
        mountain_count = len(self.get_rescue_candidates("mountain"))
        disaster_count = len(self.get_rescue_candidates("disaster"))

        return {
            "Water Rescue": water_count,
            "Mountain or Wilderness Rescue": mountain_count,
            "Disaster or Individual Tracking": disaster_count
        }
    # -------------------------------------------------------------
    # U - UPDATE
    # -------------------------------------------------------------
    def update(self, query, update_data):
        """
        Updates documents in the MongoDB collection that match the query.
        Args:
            query (dict): Key/value lookup pairs to find documents.
            update_data (dict): Key/value pairs to update in the matching documents.
        Returns:
            int: Number of documents modified.
        """
        if query is None or type(query) is not dict or update_data is None or type(update_data) is not dict:
            print("Invalid query or update data format. Must be dictionaries.")
            return 0

        try:
            result = self.collection.update_many(query, {"$set": update_data})
            return result.modified_count
        except PyMongoError as e:
            print(f"Error updating documents: {e}")
            return 0

    # -------------------------------------------------------------
    # D - DELETE
    # -------------------------------------------------------------
    def delete(self, query):
        """
        Deletes documents in the MongoDB collection that match the query.
        Args:
            query (dict): Key/value lookup pairs to find documents.
        Returns:
            int: Number of documents deleted.
        """
        if query is None or type(query) is not dict:
            print("Invalid query format. Must be a dictionary.")
            return 0

        try:
            result = self.collection.delete_many(query)
            return result.deleted_count
        except PyMongoError as e:
            print(f"Error deleting documents: {e}")
            return 0
