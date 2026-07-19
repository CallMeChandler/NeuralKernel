export default function FeatureGrid({ items = [] }) {
  return (
    <div className="feature-grid">
      {items.map((item) => (
        <article key={item.title} className="feature-card">
          <span>{item.code}</span>
          <h3>{item.title}</h3>
          <p>{item.description}</p>
        </article>
      ))}
    </div>
  );
}
